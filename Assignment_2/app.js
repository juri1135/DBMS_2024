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
    console.log("DB 연결 성공!");
});

app.use(express.json());
app.use(express.urlencoded({ extended: true }));
app.use(session({ secret: 'secret_key', resave: false, saveUninitialized: true }));

app.set("view engine", "ejs");
app.set("views", "./views");

// 전역 변수 설정
app.use((req, res, next) => {
    res.setHeader('Content-Type', 'text/html; charset=utf-8');
    res.locals.userID = req.session.userID || null;
    res.locals.state = req.session.state || 'off';
    next();
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
        'SELECT * FROM userInfo WHERE userID = ? AND password = ?', 
        [userID, password], 
        (error, results) => {
            if (error) throw error;
            if (results.length > 0) {
                // 로그인 성공 시 세션에 userID와 state 저장
                req.session.userID = userID;
                req.session.state = 'on';

                // DB에서 state를 'on'으로 업데이트
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
                        p.evalPL, 
                        p.chgPercent, 
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

                        // myInfo.ejs로 잔액과 보유 주식 현황 전달
                        res.render("myInfo", { balance, portfolio: portfolioResults });
                    }
                );
            }
        );
    } else {
        res.redirect("/login");
    }
});
// 내 정보 보기 페이지에서의 입금 처리
app.post("/deposit", (req, res) => {
    const amount = req.body.amount;
    sql_connection.query(
        'UPDATE userInfo SET balance = balance + ? WHERE userID = ?', 
        [amount, req.session.userID], 
        (error) => {
            if (error) throw error;
            // 업데이트 후 새로운 잔액 조회
            sql_connection.query(
                'SELECT balance FROM userInfo WHERE userID = ?', 
                [req.session.userID], 
                (error, results) => {
                    if (error) throw error;
                    res.json({ balance: results[0].balance });
                }
            );
        }
    );
});
// 내 정보 보기 페이지에서의  출금 처리
app.post("/withdraw", (req, res) => {
    const amount = req.body.amount;
    // 현재 잔액 확인
    sql_connection.query(
        'SELECT balance FROM userInfo WHERE userID = ?', 
        [req.session.userID], 
        (error, results) => {
            if (error) throw error;
            const currentBalance = results[0].balance;

            if (currentBalance >= amount) {
                // 출금 가능할 때 잔액 업데이트
                sql_connection.query(
                    'UPDATE userInfo SET balance = balance - ? WHERE userID = ?', 
                    [amount, req.session.userID], 
                    (error) => {
                        if (error) throw error;
                        // 업데이트 후 새로운 잔액 조회
                        sql_connection.query(
                            'SELECT balance FROM userInfo WHERE userID = ?', 
                            [req.session.userID], 
                            (error, results) => {
                                if (error) throw error;
                                res.json({ success: true, balance: results[0].balance });
                            }
                        );
                    }
                );
            } else {
                // 잔액 부족 시
                res.json({ success: false });
            }
        }
    );
});


//주식 정보 페이지
app.get("/stocks/data", (req, res) => {
    const search = req.query.search || ""; // 검색어
    const sort = req.query.sort || "";     // 정렬 조건

    // 기본 SQL 쿼리
    let query = 'SELECT stockID, curPrice, prevClose, chgPercent FROM stock WHERE stockID LIKE ?';
    const params = [`%${search}%`];

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
    }

    // 데이터베이스에서 주식 목록을 가져옴
    sql_connection.query(query, params, (error, results) => {
        if (error) throw error;
        res.json(results);
    });
});
app.get("/stocks", (req, res) => {
    res.render("stockPage");
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

    let query = `
        SELECT stockID, price, orderType, quantity, priceType, 
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


//주식 종목 별 거래 내역 페이지
app.get("/stockHistory", (req, res) => {
    const stockID = req.query.stockID;
    res.render("stockHistoryPage", { stockID });
});
app.get("/stockHistory/data", (req, res) => {
    const stockID = req.query.stockID;
    const startDate = req.query.startDate || "";
    const endDate = req.query.endDate || "";

    let query = `
        SELECT stockID, price, orderType, quantity, 
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
    let responseSent = false; // 중복 응답 방지 플래그

    // 주문 추가
    sql_connection.query(
        "INSERT INTO orderBook (userID, stockID, state, price, quantity, orderType, priceType, time) VALUES (?, ?, 'pending', ?, ?, ?, ?, ?)",
        [userID, stockID, price, quantity, orderType, priceType, time],
        (error, results) => {
            if (error) {
                if (!responseSent) { // 중복 응답 방지
                    res.status(500).json({ error: "주문 추가 실패" });
                    responseSent = true;
                }
                throw error;
            }

            // 주문 매칭 시도
            attemptOrderMatching(stockID, price, orderType, quantity, priceType === "시장가", userID, results.insertId);

            if (!responseSent) {
                res.json({ success: true, message: "주문이 추가되었습니다." });
                responseSent = true;
            }
        }
    );
});


// 거래 예약 받으면 orderBook table에 추가
app.post("/orderBook/add", (req, res) => {
    const { userID, stockID, price, quantity, orderType, priceType } = req.body;
    const state = "pending"; // 기본 상태는 'pending'
    const time = new Date();

    const query = `INSERT INTO orderBook (userID, stockID, state, price, quantity, orderType, priceType, time)
                   VALUES (?, ?, ?, ?, ?, ?, ?, ?)`;
    const params = [userID, stockID, state, price, quantity, orderType, priceType, time];

    sql_connection.query(query, params, (error, results) => {
        if (error) {
            console.error("Error inserting into orderBook:", error);
            res.status(500).json({ error: "Failed to add order to orderBook" });
            return;
        }
        res.json({ success: true, message: "Order added to orderBook" });
    });
});


// 주문 체결 이후 portfolio update balance update
function updatePortfolioAfterTrade(userID, stockID, curPrice, tradeQty, tradePrice, orderType) {
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
                    const updatePortfolioQuery = `
                        UPDATE portfolio
                        SET 
                            quantity = quantity + ?,
                            avgPrice = ((avgPrice * quantity) + (? * ?)) / (quantity + ?),
                            tradQty = tradQty + ?,
                            chgPercent = ((? - avgPrice) / avgPrice) * 100,
                            evalPL = (? - avgPrice) * quantity
                        WHERE userID = ? AND stockID = ?
                    `;
                    const params = [
                        tradeQty,
                        tradeQty, tradePrice,
                        tradeQty,
                        tradeQty,
                        curPrice,
                        curPrice,
                        userID, stockID
                    ];
                    sql_connection.query(updatePortfolioQuery, params, (updateError) => {
                        if (updateError) console.error("Error updating portfolio after buy trade:", updateError);
                        else console.log("Portfolio updated after buy trade.");
                    });
                } else if (orderType === "매도") {
                    const updatePortfolioQuery = `
                        UPDATE portfolio
                        SET 
                            quantity = quantity - ?,
                            chgPercent = ((? - avgPrice) / avgPrice) * 100,
                            evalPL = (? - avgPrice) * quantity
                        WHERE userID = ? AND stockID = ?
                    `;
                    const params = [
                        tradeQty,
                        curPrice,
                        curPrice,
                        userID, stockID
                    ];
                    sql_connection.query(updatePortfolioQuery, params, (updateError) => {
                        if (updateError) console.error("Error updating portfolio after sell trade:", updateError);
                        else {
                            console.log("Portfolio updated after sell trade.");

                            const balanceIncreaseQuery = `
                                UPDATE userInfo
                                SET balance = balance + ?
                                WHERE userID = ?
                            `;
                            const balanceIncrease = tradeQty * tradePrice;
                            sql_connection.query(balanceIncreaseQuery, [balanceIncrease, userID], (balanceError) => {
                                if (balanceError) console.error("Error updating balance after sell trade:", balanceError);
                                else console.log("Balance updated after sell trade.");
                            });
                        }
                    });
                }
            } else {
                // 신규 주식일 경우: INSERT
                const insertPortfolioQuery = `
                    INSERT INTO portfolio (userID, stockID, quantity, avgPrice, chgPercent, evalPL, tradQty)
                    VALUES (?, ?, ?, ?, ?, ?, ?)
                `;
                const params = [
                    userID,
                    stockID,
                    tradeQty,
                    tradePrice,
                    0,
                    0,
                    tradeQty
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

    // 첫 매칭 시 지정가 또는 시장가에 따라 조건 지정
    const queryCondition = isMarketOrder ? "ORDER BY price ASC, time ASC" : "AND price = ? ORDER BY time ASC";
    const params = isMarketOrder ? [stockID] : [stockID, price];

    sql_connection.query(
        `SELECT * FROM orderBook WHERE stockID = ? AND state = 'pending' AND orderType = '매도' ${queryCondition}`,
        params,
        (sellError, sellOrders) => {
            if (sellError) throw sellError;

            let remainingQuantity = quantity;
            let matchedOrders = [];
            let buyOrder = { userID, stockID, orderID, quantity, price, orderType, priceType: isMarketOrder ? '시장가' : '지정가' };

            for (const sellOrder of sellOrders) {
                const matchedQty = Math.min(sellOrder.quantity, remainingQuantity);
                remainingQuantity -= matchedQty;

                console.log(`Matched ${matchedQty} units at price ${sellOrder.price}. Remaining Quantity: ${remainingQuantity}`);
                console.log(`Sell Order Quantity After Matching: ${sellOrder.quantity}`);

                addTransactionToHistory(sellOrder, matchedQty, sellOrder.price);
                addTransactionToHistory(buyOrder, matchedQty, sellOrder.price);
                matchedOrders.push({ order: sellOrder, matchedQty });

                // 남은 수량이 없으면 매칭 완료
                if (remainingQuantity === 0) break;
            }

            // 남은 수량이 있을 경우, 더 저렴한 매도 주문을 조회하여 매칭
if (remainingQuantity > 0) {
    sql_connection.query(
        `SELECT * FROM orderBook WHERE stockID = ? AND state = 'pending' AND orderType = '매도' AND price < ? ORDER BY price DESC, time ASC`,
        [stockID, price],
        (lowerSellError, lowerSellOrders) => {
            if (lowerSellError) throw lowerSellError;

            for (const sellOrder of lowerSellOrders) {
                const matchedQty = Math.min(sellOrder.quantity, remainingQuantity);
                remainingQuantity -= matchedQty;

                console.log(`Matched ${matchedQty} units at lower price ${sellOrder.price}. Remaining Quantity: ${remainingQuantity}`);
                addTransactionToHistory(sellOrder, matchedQty, sellOrder.price);
                matchedOrders.push({ order: sellOrder, matchedQty });

                if (remainingQuantity === 0) break;
            }

            finalizeMatching(stockID, matchedOrders, buyOrder);
        }
    );
} else {
    finalizeMatching(stockID, matchedOrders, buyOrder);
}

        }
    );
}

function finalizeMatching(stockID, matchedOrders, buyOrder) {
    if (matchedOrders.length > 0) {
        updateCurrentPrice(stockID, () => {
            matchedOrders.forEach(({ order, matchedQty }) => {
                const tradeType = order.orderType === '매도' ? '매도' : '매수';
                updatePortfolioAfterTrade(order.userID, stockID, order.price, matchedQty, order.price, tradeType);
            });
            updateOrderStatus(matchedOrders, buyOrder);
        });
    } else {
        console.log("No matching sell orders found.");
    }
}


// 주문 상태 업데이트 및 수량 감소 처리
function updateOrderStatus(matchedOrders, buyOrder) {
    for (const { order, matchedQty } of matchedOrders) {
        console.log(`Updating orderID: ${order.orderID}, Original Quantity: ${order.quantity}, Matched Quantity: ${matchedQty}`);
        
        order.quantity -= matchedQty;
        console.log(`Updated Quantity after match: ${order.quantity}`);

        if (order.quantity === 0) {
            completeOrder(order.orderID);
        } else if (order.quantity > 0) {
            updatePartialOrder(order.orderID, order.quantity);
        }

        // 매칭된 수량만큼 매수 주문의 남은 수량 감소
        buyOrder.quantity -= matchedQty;
    }

    // 매수 주문의 남은 수량에 따른 처리
    if (buyOrder && buyOrder.quantity === 0) {
        completeOrder(buyOrder.orderID);
    } else if (buyOrder && buyOrder.quantity > 0) {
        updatePartialOrder(buyOrder.orderID, buyOrder.quantity);
    }
}









// 주문 완료 처리
function completeOrder(orderID) {
    sql_connection.query(
        "UPDATE orderBook SET state = 'complete' WHERE orderID = ?",
        [orderID],
        (error) => {
            if (error) throw error;
            console.log(`Order ${orderID} completed.`);
        }
    );
}

// 부분 체결 시 주문 업데이트
function updatePartialOrder(orderID, remainingQty) {
    console.log(`order: ${orderID }, remain ${remainingQty}`);
    sql_connection.query(
        "UPDATE orderBook SET quantity = ? WHERE orderID = ? AND state = 'pending'",
        [remainingQty, orderID],
        (error) => {
            if (error) throw error;
            console.log(`Order ${orderID} partially updated with remaining quantity ${remainingQty}.`);
        }
    );
}


// 체결된 거래를 transactionHistory에 기록하는 함수
function addTransactionToHistory(order, quantity, tradePrice) {
    const { userID, stockID, orderID, orderType, priceType } = order;
    const time = new Date();

    const query = `
        INSERT INTO transactionHistory (userID, stockID, orderID, price, orderType, quantity, priceType, time)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?)
    `;
    const params = [userID, stockID, orderID, tradePrice, orderType, quantity, priceType, time];

    sql_connection.query(query, params, (error, results) => {
        if (error) {
            console.error("Error adding transaction to history:", error);
            throw error;
        }
        console.log("Transaction successfully added to history.");
    });
}

// 주식 가격 업데이트 ( 수정 필요 )
function updateCurrentPrice(stockID, callback) {
    // 가장 비싼 매수 주문과 가장 저렴한 매도 주문을 조회
    sql_connection.query(
        `SELECT 
            (SELECT MAX(price) FROM orderBook WHERE stockID = ? AND state = 'pending' AND orderType = '매수') AS highestBuyPrice,
            (SELECT MIN(price) FROM orderBook WHERE stockID = ? AND state = 'pending' AND orderType = '매도') AS lowestSellPrice
        `,
        [stockID, stockID],
        (error, results) => {
            if (error) throw error;

            const highestBuyPrice = results[0].highestBuyPrice;
            const lowestSellPrice = results[0].lowestSellPrice;

            // 거래 내역에서 가장 최근의 거래 가격을 조회
            sql_connection.query(
                "SELECT price, time FROM transactionHistory WHERE stockID = ? ORDER BY time DESC LIMIT 1",
                [stockID],
                (historyError, historyResults) => {
                    if (historyError) throw historyError;

                    const latestTransactionPrice = historyResults.length ? historyResults[0].price : null;
                    const latestTransactionTime = historyResults.length ? historyResults[0].time : null;

                    // 최신 가격 결정
                    let newPrice;
                    if (latestTransactionPrice !== null) {
                        // 가장 최근의 거래 가격이 매수/매도 가격과 비교되어야 함
                        if (highestBuyPrice !== null && lowestSellPrice !== null) {
                            // 둘 중에서 최신 거래 가격이 비싼 매수 또는 싼 매도보다 더 최신인 경우 해당 가격 선택
                            newPrice = latestTransactionTime >= Math.max(highestBuyPrice, lowestSellPrice)
                                ? latestTransactionPrice
                                : Math.max(highestBuyPrice, lowestSellPrice);
                        } else if (highestBuyPrice !== null) {
                            newPrice = Math.max(latestTransactionPrice, highestBuyPrice);
                        } else if (lowestSellPrice !== null) {
                            newPrice = Math.min(latestTransactionPrice, lowestSellPrice);
                        } else {
                            newPrice = latestTransactionPrice; // 매수/매도 주문이 없는 경우
                        }
                    } else {
                        newPrice = highestBuyPrice !== null ? highestBuyPrice : lowestSellPrice;
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
                        if (callback) callback(); // 가격이 없는 경우에도 callback 호출
                    }
                }
            );
        }
    );
}









// orderBook 기반으로 5단계 호가창 만들기
app.get("/orderBook/quote", (req, res) => {
    const stockID = req.query.stockID;

    const query = `
        SELECT price, SUM(quantity) AS totalQuantity, orderType
        FROM orderBook
        WHERE stockID = ? AND state = 'pending'
        GROUP BY price, orderType
        ORDER BY price ASC
    `;

    sql_connection.query(query, [stockID], (error, results) => {
        if (error) {
            console.error("Error fetching quote data:", error);
            res.status(500).json({ error: "Failed to fetch quote data" });
            return;
        }

        const sellOrders = results.filter(order => order.orderType === '매도').slice(0, 5);
        const buyOrders = results.filter(order => order.orderType === '매수').slice(-5).reverse();

        res.json({ sellOrders, buyOrders });
    });
});
// 호가창
app.get("/quotePage", (req, res) => {
    const stockID = req.query.stockID;

    const sellQuery = `
        SELECT * FROM (
            SELECT price AS sellPrice, SUM(quantity) AS sellQty
            FROM orderBook
            WHERE stockID = ? AND state = 'pending' AND orderType = '매도'
            GROUP BY price
            ORDER BY price ASC
            LIMIT 5
        ) AS LowestPrices
        ORDER BY sellPrice DESC
    `;

    const buyQuery = `
        SELECT price AS buyPrice, SUM(quantity) AS buyQty
        FROM orderBook
        WHERE stockID = ? AND state = 'pending' AND orderType = '매수'
        GROUP BY price
        ORDER BY price DESC
        LIMIT 5
    `;

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






// 거래 말고 주문 내역 보는 페이지
app.get('/orderHistory', (req, res) => {
    const userID = req.session.userID;

    sql_connection.query(
        "SELECT stockID, quantity, price, price * quantity AS totalPrice, orderType, priceType, DATE_FORMAT(time, '%Y-%m-%d %H:%i:%s') AS formattedTime, orderID FROM orderBook WHERE userID = ? AND state = 'pending' ORDER BY time DESC",
        [userID],
        (error, orders) => {
            if (error) {
                console.error("Error fetching order history:", error);
                return res.status(500).send("Internal Server Error");
            }
            console.log("Orders fetched:", orders); // orders 확인
            res.render('orderHistory', { orders });
        }
    );
});

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

    // 디버깅용 로그 추가
    console.log("Received update balance request:");
    console.log("userID:", userID);
    console.log("balanceChange:", balanceChange);

    if (!userID || balanceChange == null) {
        console.error("Invalid data received for balance update");
        return res.status(400).json({ success: false, error: "Invalid data" });
    }

    const query = `UPDATE userInfo SET balance = balance + ? WHERE userID = ?`;
    sql_connection.query(query, [balanceChange, userID], (error, results) => {
        if (error) {
            console.error("Balance update error:", error);
            return res.status(500).json({ success: false, error: "Balance update error" });
        }
        res.json({ success: true });
    });
});


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








// userInfo에서 예수금 update하기 (매수 예약 or 매도 체결)
app.post('/userInfo/updateBalance', (req, res) => {
    const { userID, balanceChange } = req.body;
    const query = `UPDATE userInfo SET balance = balance + ? WHERE userID = ?`;
    sql_connection.query(query, [balanceChange, userID], (error, results) => {
        if (error) {
            console.error('Balance update error:', error);
            res.json({ success: false });
        } else {
            res.json({ success: true });
        }
    });
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
    const query = `UPDATE portfolio SET tradQty = tradQty + ? WHERE userID = ? AND stockID = ?`;
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
