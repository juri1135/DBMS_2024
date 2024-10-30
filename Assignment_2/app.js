
const express = require('express');
const app = express();
const router = express.Router();
const mysql = require('mysql2');
const db_info = {
    host: "localhost",
    port: "3306",
    user: "root",
    password: "Gjuri0917^^",
    database: "appdevexample"
};
const sql_connection = mysql.createConnection(db_info);
sql_connection.connect();

app.use(express.json());
app.use(express.urlencoded({ extended: true }));

//ejs는 package임
app.set("view engine", "ejs");
//views라는 폴더에 있는 것들이 실제 user에게 보여지는 것들
app.set("views", "./views");

app.use("/", router);

router.route("/")
     .get((req, res) => {
        res.render("app"); // "app" 템플릿 렌더링 (app 객체가 아님)
    });


app.get
    ("/login", (req, res) => {
    res.render("loginPage")
    })
app.post
    ("/login", (req, res) => {
        const { userID, userPassword } = req.body;
        sql_connection.query('SELECT * FROM userinfo WHERE userID=? AND userPassword=?', [userID,userPassword], (error, results, fields) => {
            if (error) throw error;
            if (results.length > 0) {
                res.send('로그인 성공')
                
            }
            else {
                    res.render("loginPage",{loginResult:'fail'})
                
            }
        })
    })

app.get
    ("/signup", (req, res) => {
    res.render("signupPage")
    })
app.post
    ("/signup", (req, res) => {
        const { userID, userPassword } = req.body;
        sql_connection.query('SELECT * FROM userinfo WHERE userID=?', [userID], (error, results, fields) => {
            if (error) throw error;
            if (results.length > 0) {
                res.send("이미 존재하는 아이디입니다.")
            }
            else {
                sql_connection.query('INSERT INTO userinfo(userID, userPassword) values(?,?)', [userID, userPassword], (error, results, fields) => {
                if (error) throw error;
                else {
                    res.send("회원가입 완료.")
                }
                })
            }
        })
    })
//port 3000을 듣겠다 + callback 함수
app.listen(3000, () => {
    console.log(
        '서버 실행중'
    )
});

