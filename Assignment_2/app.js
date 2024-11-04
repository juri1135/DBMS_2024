const express = require('express');
const app = express();
const mysql = require('mysql2');
const session = require('express-session');
const db_info = {
    host: "localhost",
    port: "3306",
    user: "root",
    password: "Gjuri0917^^",
    database: "DB_2022056262"
};
const sql_connection = mysql.createConnection(db_info);
sql_connection.connect((err) => {
    if (err) throw err;
});

app.use(express.json());
app.use(express.urlencoded({ extended: true }));
app.use(session({ secret: 'secret_key', resave: false, saveUninitialized: true }));

app.set("view engine", "ejs");
app.set("views", "./views");
const cron = require('node-cron');
// 전역 변수 설정
let todayDate = new Date().toISOString().split('T')[0];
let totalDeductedAmount = 0; // 차감된 가격, 초기값은 0
let totalTransactionPrice = 0;
app.use((req, res, next) => {
    res.setHeader('Content-Type', 'text/html; charset=utf-8');
    res.locals.userID = req.session.userID || null;
    res.locals.state = req.session.state || 'off';
    res.locals.todayDate = todayDate;
    next();
});

// 매일 자정에 실행
cron.schedule('0 0 * * *', () => {
    // 자정에 오늘 날짜 갱신
    todayDate = new Date().toISOString().split('T')[0];

    // 주식 테이블에서 prevClose를 curPrice로 업데이트
    sql_connection.query(
        `UPDATE stock 
         SET prevClose = curPrice`,
        (error) => {
            if (error) throw error;
            console.log('prevClose updated successfully');
        }
    );
});

// 기본 페이지 (로그인/회원가입 또는 환영 메시지와 로그아웃 버튼)
app.get("/", (req, res) => {
    res.render("app");
});

// 회원가입 페이지
app.get("/signup", (req, res) => {
    res.render("signupPage", { signupResult: null });
});

app.post("/signup", (req, res) => {
    const { userID, userPassword } = req.body;
    sql_connection.query('SELECT * FROM userInfo WHERE userID=?', [userID], (error, results) => {
        if (error) throw error;
        if (results.length > 0) {
            res.render("signupPage", { signupResult: 'fail' });
        } else {
            sql_connection.query(
                'INSERT INTO userInfo(userID, password, state, balance) VALUES (?, ?, "off", 0)', 
                [userID, userPassword], 
                (error, results) => {
                    if (error) throw error;
                    res.render("signupPage", { signupResult: 'success' });  // 회원가입 성공 메시지 표시
                }
            );
        }
    });
});


// 로그인 페이지
app.get("/login", (req, res) => {
    res.render("loginPage", { loginResult: null });
});

app.post("/login", (req, res) => {
    const { userID, password } = req.body;
    sql_connection.query(
        'SELECT * FROM userInfo WHERE userID = ? AND password = ? AND state!="del"', 
        [userID, password], 
        (error, results) => {
            if (error) throw error;
            if (results.length > 0) {
                req.session.userID = userID;
                req.session.state = 'on';
                sql_connection.query(
                    'UPDATE userInfo SET state = "on" WHERE userID = ?', 
                    [userID],
                    (error) => {
                        if (error) throw error;
                        res.redirect('/');
                    }
                );
            } else {
                res.render("loginPage", { loginResult: 'fail' });
            }
        }
    );
});
// 로그아웃 라우트
app.post("/logout", (req, res) => {
    const userID = req.session.userID;
    // DB에서 state를 'off'로 업데이트
    sql_connection.query(
        'UPDATE userInfo SET state = "off" WHERE userID = ?', 
        [userID],
        (error) => {
            if (error) throw error;

            // 세션 정보 삭제
            req.session.destroy((err) => {
                if (err) throw err;
                res.redirect('/');
            });
        }
    );
});
// 탈퇴
app.post("/idDel", (req, res) => {
    const userID = req.session.userID;
    sql_connection.query(
        'UPDATE userInfo SET state = "del" WHERE userID = ?', 
        [userID],
        (error) => {
            if (error) throw error;

            req.session.destroy((err) => {
                if (err) throw err;
                res.redirect('/');
            });
        }
    );
});


//내 정보 보기 페이지
app.get("/myInfo", (req, res) => {
    if (req.session.state === 'on') {
        const userID = req.session.userID;

        // 잔액 조회와 보유 주식 현황 조회
        sql_connection.query(
            'SELECT balance FROM userInfo WHERE userID = ?', 
            [userID], 
            (error, results) => {
                if (error) throw error;
                const balance = results[0].balance;

                // 보유 주식 현황 조회
                sql_connection.query(
                    `SELECT 
                        p.stockID, 
                        p.avgPrice, 
                        s.curPrice, 
                        p.quantity, 
                        p.tradQty
                     FROM 
                        portfolio p
                     JOIN 
                        stock s ON p.stockID = s.stockID
                     WHERE 
                        p.userID = ?`, 
                    [userID],
                    (error, portfolioResults) => {
                        if (error) throw error;

                        // 평가손익과 등락률 계산
                        const updatedPortfolio = portfolioResults.map(stock => {
                            const chgPercent = ((stock.curPrice - stock.avgPrice) / stock.avgPrice) * 100;
                            const evalPL = (stock.curPrice - stock.avgPrice) * stock.quantity;
                            return {
                                ...stock,
                                chgPercent: chgPercent.toFixed(2), // 소수점 2자리로 변환
                                evalPL: evalPL.toFixed(2) // 소수점 2자리로 변환
                            };
                        });

                        // myInfo.ejs로 잔액과 보유 주식 현황 전달
                        res.render("myInfo", { balance, portfolio: updatedPortfolio });
                    }
                );
            }
        );
    } else {
        res.redirect("/login");
    }
});






//거래 내역 페이지
app.get("/transactions", (req, res) => {
    res.render("transactionPage");
});
app.get("/transactions/data", (req, res) => {
    const userID = req.session.userID; // 세션에서 사용자 ID 가져오기
    const stockSearch = req.query.stockSearch || "";
    const startDate = req.query.startDate || "";
    const endDate = req.query.endDate || "";

    let query = 
        `SELECT stockID, price, orderType, quantity, priceType, 
               DATE_FORMAT(time, '%Y-%m-%d %H:%i:%s') AS time 
        FROM transactionHistory 
        WHERE userID = ? AND stockID LIKE ?`;

    const params = [userID, `%${stockSearch}%`];

    if (startDate) {
        query += " AND time >= ?";
        params.push(startDate);
    }
    if (endDate) {
        query += " AND time <= ?";
        params.push(endDate);
    }

    sql_connection.query(query, params, (error, results) => {
        if (error) throw error;
        res.json(results);
    });
});


// 거래 말고 주문 내역 보는 페이지
app.get('/orderHistory', (req, res) => {
    const userID = req.session.userID;

    sql_connection.query(
        "SELECT stockID, quantity, price, price * quantity AS totalPrice, orderType, priceType, DATE_FORMAT(time, '%Y-%m-%d %H:%i:%s') AS formattedTime, orderID FROM orderBook WHERE userID = ? AND state = 'pending' ORDER BY time ASC",
        [userID],
        (error, orders) => {
            if (error) {
                console.error("Error fetching order history:", error);
                return res.status(500).send("Internal Server Error");
            }
            res.render('orderHistory', { orders });
        }
    );
});



//주식 정보 페이지
app.get("/stocks/data", (req, res) => {
    const search = req.query.search || ""; // 검색어
    const sort = req.query.sort || "";     // 정렬 조건

    // 기본 SQL 쿼리 (거래량 추가)
    let query = `
        SELECT stock.stockID, 
               stock.curPrice, 
               stock.prevClose, 
               ((stock.curPrice - stock.prevClose) / stock.prevClose) * 100 AS chgPercent,
               IFNULL(SUM(th.quantity), 0) AS tradeVolume
        FROM stock
        LEFT JOIN transactionHistory th ON stock.stockID = th.stockID 
          AND DATE(th.time) = ?  -- 오늘 날짜와 일치하는 거래만 계산
        WHERE stock.stockID LIKE ?
        GROUP BY stock.stockID, stock.curPrice, stock.prevClose
    `;

    const params = [todayDate, `%${search}%`];

    // 정렬 조건 추가
    switch (sort) {
        case "chgPercent_desc":
            query += " ORDER BY chgPercent DESC";
            break;
        case "chgPercent_asc":
            query += " ORDER BY chgPercent ASC";
            break;
        case "curPrice_desc":
            query += " ORDER BY curPrice DESC";
            break;
        case "curPrice_asc":
            query += " ORDER BY curPrice ASC";
            break;
        case "prevClose_desc":
            query += " ORDER BY prevClose DESC";
            break;
        case "prevClose_asc":
            query += " ORDER BY prevClose ASC";
            break;
        case "tradeVolume_desc":
            query += " ORDER BY tradeVolume DESC";
            break;
        case "tradeVolume_asc":
            query += " ORDER BY tradeVolume ASC";
            break;
    }

    // 데이터베이스에서 주식 목록을 가져옴
    sql_connection.query(query, params, (error, results) => {
        if (error) {
            console.error("Error fetching stocks:", error);
            res.status(500).json({ error: "Failed to fetch stocks" });
            return;
        }
        res.json(results);
    });
});

app.get("/stocks", (req, res) => {
    res.render("stockPage");
});



//주식 종목 별 거래 내역 페이지
app.get("/stockHistory", (req, res) => {
    const stockID = req.query.stockID;
    res.render("stockHistoryPage", { stockID });
});
app.get("/stockHistory/data", (req, res) => {
    const stockID = req.query.stockID;
    const startDate = req.query.startDate || "";
    const endDate = req.query.endDate || "";

    let query = 
        `SELECT stockID, price, orderType, quantity, 
               DATE_FORMAT(time, '%Y-%m-%d %H:%i:%s') AS time 
        FROM transactionHistory 
        WHERE stockID = ?`;

    const params = [stockID];

    if (startDate) {
        query += " AND time >= ?";
        params.push(startDate);
    }
    if (endDate) {
        query += " AND time <= ?";
        params.push(endDate);
    }

    sql_connection.query(query, params, (error, results) => {
        if (error) throw error;
        res.json(results);
    });
});




// 호가창
app.get("/quotePage", (req, res) => {
    const stockID = req.query.stockID;

    const sellQuery = 
        `SELECT * FROM (
            SELECT price AS sellPrice, SUM(quantity) AS sellQty
            FROM orderBook
            WHERE stockID = ? AND state = 'pending' AND orderType = '매도'
            GROUP BY price
            ORDER BY price ASC
            LIMIT 5
        ) AS LowestPrices
        ORDER BY sellPrice DESC`
    ;

    const buyQuery = 
        `SELECT price AS buyPrice, SUM(quantity) AS buyQty
        FROM orderBook
        WHERE stockID = ? AND state = 'pending' AND orderType = '매수'
        GROUP BY price
        ORDER BY price DESC
        LIMIT 5`
    ;

    sql_connection.query(sellQuery, [stockID], (sellError, sellResults) => {
        if (sellError) throw sellError;

        sql_connection.query(buyQuery, [stockID], (buyError, buyResults) => {
            if (buyError) throw buyError;

            const sellSum = sellResults.reduce((acc, row) => acc + Number(row.sellQty), 0);
            const buySum = buyResults.reduce((acc, row) => acc + Number(row.buyQty), 0);

            res.render("quotePage", {
                stockID,
                sellOrders: sellResults,
                buyOrders: buyResults,
                sellSum,
                buySum
            });
        });
    });
});




// 거래하기 페이지
app.get("/trade", (req, res) => {
    const stockID = req.query.stockID; // 쿼리에서 stockID를 가져옴

    // stock 테이블에서 stockID에 해당하는 현재가(curPrice)를 가져오기
    const query = "SELECT curPrice FROM stock WHERE stockID = ?";
    sql_connection.query(query, [stockID], (error, results) => {
        if (error) {
            console.error("Error fetching stock data:", error);
            res.status(500).send("Internal Server Error");
            return;
        }

        // stockID가 유효할 경우, 해당 정보를 전달하여 페이지 렌더링
        if (results.length > 0) {
            const curPrice = results[0].curPrice;
            res.render("tradePage", { stockID, curPrice });
        } else {
            res.status(404).send("Stock not found");
        }
    });
});
// 주문 추가 및 매칭 로직
app.post("/trade", (req, res) => {
    const { userID, stockID, price, quantity, orderType, priceType } = req.body;
    const time = new Date();
    if (orderType === "매수") {
        totalDeductedAmount = price * quantity; // 매수 시 차감금 설정
    }
    // 주문 추가
    sql_connection.query(
        "INSERT INTO orderBook (userID, stockID, state, price, quantity, orderType, priceType, time) VALUES (?, ?, 'pending', ?, ?, ?, ?, ?)",
        [userID, stockID, price, quantity, orderType, priceType, time],
        (error, results) => {
            if (error) {throw error;}

            // 주문 매칭 시도
            attemptOrderMatching(stockID, price, orderType, quantity, priceType === "시장가", userID, results.insertId);
            res.json({ success: true, message: "Order added to orderBook", totalDeductedAmount });
        }
    );
});





// 주문 체결 이후 portfolio update balance update
function updatePortfolioAfterTrade(userID, stockID, tradePrice, matchedQty, orderType) {
    console.log("Portfolio update triggered with:", {
        userID, stockID, tradePrice, matchedQty, orderType
    });

    const tradeAmount = tradePrice * matchedQty;

    sql_connection.query(
        "SELECT * FROM portfolio WHERE userID = ? AND stockID = ?",
        [userID, stockID],
        (error, results) => {
            if (error) {
                console.error("Error checking portfolio existence:", error);
                return;
            }

            if (results.length > 0) {
                // 주식이 이미 포트폴리오에 있는 경우: UPDATE
                if (orderType === "매수") {
                    const updatePortfolioQuery = 
                        `UPDATE portfolio
                        SET 
                            avgPrice = ((avgPrice * quantity) + (? * ?)) / (quantity + ?),
                            quantity = quantity + ?,
                            tradQty = tradQty + ?,
                            chgPercent = ((? - avgPrice) / avgPrice) * 100,
                            evalPL = (? - avgPrice) * quantity
                        WHERE userID = ? AND stockID = ?`
                    ;
                    const params = [
                        matchedQty, tradePrice, // 체결된 거래 가격 사용
                        matchedQty,
                        matchedQty,
                        matchedQty,
                        tradePrice, // 체결된 거래 가격 사용
                        tradePrice, // 체결된 거래 가격 사용
                        userID, stockID
                    ];
                    sql_connection.query(updatePortfolioQuery, params, (updateError) => {
                        if (updateError) console.error("Error updating portfolio after buy trade:", updateError);
                        else console.log("Portfolio updated after buy trade.");
                    });
                } else if (orderType === "매도") {
                    const updatePortfolioQuery = 
                        `UPDATE portfolio
                        SET 
                            quantity = quantity - ?,
                            chgPercent = ((? - avgPrice) / avgPrice) * 100,
                            evalPL = (? - avgPrice) * quantity
                        WHERE userID = ? AND stockID = ?`
                    ;
                    const params = [
                        matchedQty,
                        tradePrice, // 체결된 거래 가격 사용
                        tradePrice, // 체결된 거래 가격 사용
                        userID, stockID
                    ];
                    sql_connection.query(updatePortfolioQuery, params, (updateError) => {
                        if (updateError) console.error("Error updating portfolio after sell trade:", updateError);
                        else console.log("Portfolio updated after sell trade.");
                    });
                }
            } else {
                // 신규 주식일 경우: INSERT
                const insertPortfolioQuery = 
                    `INSERT INTO portfolio (userID, stockID, quantity, avgPrice, chgPercent, evalPL, tradQty)
                    VALUES (?, ?, ?, ?, ?, ?, ?)`
                ;
                const params = [
                    userID,
                    stockID,
                    matchedQty,
                    tradePrice, // 체결된 거래 가격 사용
                    0,
                    0,
                    matchedQty
                ];
                sql_connection.query(insertPortfolioQuery, params, (insertError) => {
                    if (insertError) console.error("Error inserting new stock into portfolio:", insertError);
                    else console.log("New stock added to portfolio.");
                });
            }
        }
    );
}




function attemptOrderMatching(stockID, price, orderType, quantity, isMarketOrder = false, userID, orderID) {
    console.log("Attempting to match orders for", stockID, "at price:", price, "Order type:", orderType, "Market order:", isMarketOrder);

    const matchingOrderType = orderType === "매수" ? "매도" : "매수";
    let queryCondition, params;

    if (!isMarketOrder) {
        queryCondition = orderType === "매수"
            ? "AND price <= ? ORDER BY price DESC, time ASC"
            : "AND price >= ? ORDER BY price ASC, time ASC";
        params = [stockID, matchingOrderType, price];
    } else {
        queryCondition = orderType === "매수"
            ? "ORDER BY price ASC, time ASC"
            : "ORDER BY price DESC, time ASC";
        params = [stockID, matchingOrderType];
    }

    sql_connection.query(
        `SELECT * FROM orderBook WHERE stockID = ? AND state = 'pending' AND orderType = ? ${queryCondition}`,
        params,
        (error, initialOrders) => {
            if (error) throw error;

            console.log("Matched orders:", initialOrders);
            let remainingQuantity = quantity;
            let matchedOrders = [];
            let currentOrder = { userID, stockID, orderID, quantity, price, orderType, priceType: isMarketOrder ? '시장가' : '지정가' };

            for (const order of initialOrders) {
                const matchedQty = Math.min(order.quantity, remainingQuantity);
                remainingQuantity -= matchedQty;

                totalTransactionPrice += matchedQty * order.price;
                console.log(`total(same): ${totalTransactionPrice}`);
                
                addTransactionToHistory(order, matchedQty, order.price);
                addTransactionToHistory(currentOrder, matchedQty, order.price);

                // 매도자의 잔액 업데이트
                if (order.orderType === "매도") {
                    updateWallet(order.userID, matchedQty * order.price);
                }

                matchedOrders.push({ order, matchedQty });

                if (remainingQuantity === 0) break;
            }

            finalizeMatching(stockID, matchedOrders, currentOrder);
        }
    );
}






function finalizeMatching(stockID, matchedOrders, currentOrder) {
    console.log("Finalizing matching for stock:", stockID);
    
    
    if (matchedOrders.length > 0) {
        updateOrderStatus(matchedOrders, currentOrder, () => {
            updateCurrentPrice(stockID, () => {
                // 현재가 업데이트 후 포트폴리오 업데이트
                matchedOrders.forEach(({ order, matchedQty }) => {
                    const tradeType = order.orderType === "매도" ? "매도" : "매수";
                    
                    // 수정된 호출, matchedQty로 명확히 전달
                    updatePortfolioAfterTrade(order.userID, stockID, order.price, matchedQty, tradeType);
                    
                    updatePortfolioAfterTrade(currentOrder.userID, stockID, currentOrder.price, matchedQty, currentOrder.orderType === "매수" ? "매수" : "매도");
                });

                if (currentOrder.orderType === "매수") {
                    const balanceAdjustment = totalDeductedAmount - totalTransactionPrice;
                    updateWallet(currentOrder.userID, balanceAdjustment * -1);
                }
            });
        });
    } else {
        console.log("No matched orders found, updating current price...");
        updateCurrentPrice(stockID, () => {});
    }
}

function updateWallet(userID, amount) {
    sql_connection.query(
        "UPDATE userInfo SET balance = balance + ? WHERE userID = ?",
        [amount, userID],
        (error) => {
            if (error) console.error("Error updating balance:", error);
        }
    );
}

// 주문 상태 업데이트 및 수량 감소 처리
function updateOrderStatus(matchedOrders, buyOrder, callback) {
    
    for (const { order, matchedQty } of matchedOrders) {
        order.quantity -= matchedQty;

        if (order.quantity === 0) {
            completeOrder(order.orderID, (err) => {
                if (err) {

                    console.error("Error completing order:", err);
                    return callback(err);  // 에러 발생 시 콜백 호출
                }
                console.log(`Order ${order.orderID} completed`);
            });
        } else if (order.quantity > 0) {
            updatePartialOrder(order.orderID, order.quantity, (err) => {
                if (err) {
                    console.error("Error updating partial order:", err);
                    return callback(err);  // 에러 발생 시 콜백 호출
                }
                console.log(`Order ${order.orderID} partially updated with remaining quantity ${order.quantity}`);
            });
        }

        // 매칭된 수량만큼 매수 주문의 남은 수량 감소
        buyOrder.quantity -= matchedQty;
    }

    // 매수 주문의 남은 수량에 따른 처리
    if (buyOrder && buyOrder.quantity === 0) {
        completeOrder(buyOrder.orderID, (err) => {
            if (err) {
                console.error("Error completing buy order:", err);
                return callback(err);  // 에러 발생 시 콜백 호출
            }
            console.log(`Buy order ${buyOrder.orderID} completed`);
            callback(null);  // 정상 완료 시 콜백 호출
        });
    } else if (buyOrder && buyOrder.quantity > 0) {
        updatePartialOrder(buyOrder.orderID, buyOrder.quantity, (err) => {
            if (err) {
                console.error("Error updating partial buy order:", err);
                return callback(err);  // 에러 발생 시 콜백 호출
            }
            console.log(`Buy order ${buyOrder.orderID} partially updated with remaining quantity ${buyOrder.quantity}`);
            callback(null);  // 정상 완료 시 콜백 호출
        });
    } else {
        callback(null);  // 매칭할 주문이 없을 경우에도 콜백 호출
    }
}





// 주문 완료 처리
function completeOrder(orderID, callback) {
    sql_connection.query(
        "UPDATE orderBook SET state = 'complete' WHERE orderID = ?",
        [orderID],
        (error, results) => {
            if (error) {
                console.error("Error completing order in database:", error);
                return callback(error); // 에러 발생 시 콜백에 전달
            }
            console.log(`Order ${orderID} marked as complete in database.`);
            callback(null); // 정상적으로 완료되었을 때 콜백 호출
        }
    );
}


// 부분 체결 시 주문 업데이트
function updatePartialOrder(orderID, remainingQty) {
   sql_connection.query(
        "UPDATE orderBook SET quantity = ? WHERE orderID = ? AND state = 'pending'",
        [remainingQty, orderID],
        (error) => {
            if (error) throw error;
        }
    );
}


// 체결된 거래를 transactionHistory에 기록하는 함수
function addTransactionToHistory(order, quantity, tradePrice) {
    const { userID, stockID, orderID, orderType, priceType } = order;
    const time = new Date();

    const query = 
        `INSERT INTO transactionHistory (userID, stockID, orderID, price, orderType, quantity, priceType, time)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?)`
    ;
    const params = [userID, stockID, orderID, tradePrice, orderType, quantity, priceType, time];

    sql_connection.query(query, params, (error, results) => {
        if (error) {
            console.error("Error adding transaction to history:", error);
            throw error;
        }
    });
}

// 주식 가격 업데이트 ( 수정 필요 )
function updateCurrentPrice(stockID, callback) {
    // 매수 최고가와 매도 최저가 조회
    sql_connection.query(
        `SELECT 
            (SELECT MAX(price) FROM orderBook WHERE stockID = ? AND state = 'pending' AND orderType = '매수') AS highestBuyPrice,
            (SELECT MIN(price) FROM orderBook WHERE stockID = ? AND state = 'pending' AND orderType = '매도') AS lowestSellPrice`,
        [stockID, stockID],
        (error, results) => {
            if (error) throw error;

            const highestBuyPrice = results[0].highestBuyPrice;
            const lowestSellPrice = results[0].lowestSellPrice;

            // 매수 최고가와 매도 최저가에 일치하는 거래 내역을 조회
            sql_connection.query(
                `SELECT price, time FROM transactionHistory 
                 WHERE stockID = ? AND (price = ? OR price = ?)
                 ORDER BY time DESC`,
                [stockID, highestBuyPrice, lowestSellPrice],
                (historyError, historyResults) => {
                    if (historyError) throw historyError;

                    let latestTransactionPrice = null;
                    let latestTransactionTime = null;

                    // 매수 최고가 또는 매도 최저가와 일치하는 가장 최근 거래 내역 찾기
                    for (const transaction of historyResults) {
                        if (transaction.price === highestBuyPrice || transaction.price === lowestSellPrice) {
                            latestTransactionPrice = transaction.price;
                            latestTransactionTime = transaction.time;
                            break; // 가장 최근 거래 내역만 필요하므로 중지
                        }
                    }

                    // 새로운 현재가 결정 로직
                    let newPrice;
                    if (latestTransactionPrice === highestBuyPrice) {
                        newPrice = highestBuyPrice;
                    } else if (latestTransactionPrice === lowestSellPrice) {
                        newPrice = lowestSellPrice;
                    } else if (highestBuyPrice !== null && lowestSellPrice !== null) {
                        newPrice = (highestBuyPrice + lowestSellPrice) / 2;
                    } else {
                        newPrice = highestBuyPrice || lowestSellPrice;
                    }

                    if (newPrice !== null) {
                        // 주식 테이블에 현재가 업데이트
                        sql_connection.query(
                            "UPDATE stock SET curPrice = ? WHERE stockID = ?",
                            [newPrice, stockID],
                            (updateError) => {
                                if (updateError) throw updateError;
                                console.log("Current price successfully updated to", newPrice);

                                // 가격 업데이트 이후에 포트폴리오 업데이트 진행
                                if (callback) callback();
                            }
                        );
                    } else {
                        console.log("No price to update for current price.");
                        if (callback) callback();
                    }
                }
            );
        }
    );
}












// 특정 주문 정보 가져오기
app.get('/orderBook/getOrderDetails', (req, res) => {
    const orderID = req.query.orderID;

    sql_connection.query(
        "SELECT userID, stockID, orderType, quantity, price FROM orderBook WHERE orderID = ?",
        [orderID],
        (error, results) => {
            if (error) {
                console.error("Error fetching order details:", error);
                return res.status(500).json({ success: false, error: "Error fetching order details" });
            }
            res.json(results[0]);
        }
    );
});

// 주문 삭제
app.post('/orderBook/deleteOrder', (req, res) => {
    const { orderID } = req.body;

    sql_connection.query(
        "DELETE FROM orderBook WHERE orderID = ?",
        [orderID],
        (error) => {
            if (error) {
                console.error("Error deleting order:", error);
                return res.status(500).json({ success: false, error: "Error deleting order" });
            }
            res.json({ success: true });
        }
    );
});

// balance 업데이트
app.post('/userInfo/updateBalance', (req, res) => {
    const { userID, balanceChange } = req.body;
    if (balanceChange < 0) {
        // 출금 요청일 때 (balanceChange가 음수)
        sql_connection.query(
            'SELECT balance FROM userInfo WHERE userID = ?', 
            [userID], 
            (error, results) => {
                if (error) {
                    console.error("Error checking balance:", error);
                    return res.status(500).json({ success: false, error: "Balance check error" });
                }
                console.log(results[0]?.balance);
                const currentBalance = results[0]?.balance;
                if (currentBalance !== undefined && currentBalance >= Math.abs(balanceChange)) {
                    // 잔액이 충분할 때 출금
                    updateBalance(userID, balanceChange, res);
                } else {
                    // 잔액이 부족할 때
                    res.json({ success: false, error: "Insufficient balance" });
                }
            }
        );
    } else {
        // 입금 요청일 때 (balanceChange가 양수)
        updateBalance(userID, balanceChange, res);
    }
});

function updateBalance(userID, balanceChange, res) {
    const updateQuery = 'UPDATE userInfo SET balance = balance + ? WHERE userID = ?';
    
    sql_connection.query(updateQuery, [balanceChange, userID], (error) => {
        if (error) {
            console.error("Balance update error:", error);
            return res.status(500).json({ success: false, error: "Balance update error" });
        }

        // 업데이트 후 새로운 잔액 조회
        const balanceQuery = "SELECT balance FROM userInfo WHERE userID = ?";
        sql_connection.query(balanceQuery, [userID], (error, results) => {
            if (error) {
                console.error("Error fetching balance:", error);
                return res.status(500).json({ success: false, error: "Failed to fetch balance" });
            }

            if (results.length > 0) {
                res.json({ success: true, balance: results[0].balance });
            } else {
                res.status(404).json({ success: false, error: "User not found" });
            }
        });
    });
}

// tradQty 업데이트
app.post('/portfolio/updateTradableQty', (req, res) => {
    const { userID, stockID, quantity } = req.body;

    sql_connection.query(
        "UPDATE portfolio SET tradQty = tradQty + ? WHERE userID = ? AND stockID = ?",
        [quantity, userID, stockID],
        (error) => {
            if (error) {
                console.error("Error updating tradable quantity:", error);
                return res.status(500).json({ success: false, error: "Error updating tradable quantity" });
            }
            res.json({ success: true });
        }
    );
});

// userinfo table에서 balance 받아 오기
app.get("/userInfo/balance", (req, res) => {
    const userID = req.session.userID;

    const query = "SELECT balance FROM userInfo WHERE userID = ?";
    sql_connection.query(query, [userID], (error, results) => {
        if (error) {
            console.error("Error fetching balance:", error);
            res.status(500).json({ error: "Failed to fetch balance" });
            return;
        }

        if (results.length > 0) {
            res.json({ balance: results[0].balance });
        } else {
            res.status(404).json({ error: "User not found" });
        }
    });
});
// portfolio table에서 거래 가능 수량 받아 오기
app.get("/portfolio/tradableQty", (req, res) => {
    const stockID = req.query.stockID;
    const userID = req.session.userID; // 사용자가 로그인되어 있어야 함

    const query = "SELECT tradQty FROM portfolio WHERE userID = ? AND stockID = ?";
    sql_connection.query(query, [userID, stockID], (error, results) => {
        if (error) {
            console.error("Error fetching tradable quantity:", error);
            res.status(500).json({ error: "Failed to fetch tradable quantity" });
            return;
        }

        if (results.length > 0) {
            res.json({ quantity: results[0].tradQty });
        } else {
            res.status(404).json({ error: "Portfolio entry not found" });
        }
    });
});
// portfolio에서 거래 가능 수량 update하기 (매도 예약 or 매수 체결)
app.post('/portfolio/updateTradableQty', (req, res) => {
    const { userID, stockID, quantityChange } = req.body;
    const query =" UPDATE portfolio SET tradQty = tradQty + ? WHERE userID = ? AND stockID = ?";
    sql_connection.query(query, [quantityChange, userID, stockID], (error, results) => {
        if (error) {
            console.error('Tradable quantity update error:', error);
            res.json({ success: false });
        } else {
            res.json({ success: true });
        }
    });
});

app.listen(3000, () => {
    console.log("서버 실행 중");
});
