-- Active: 1727165140235@@127.0.0.1@3306@maple_database
--이름이 J로 시작하지 않는 유저의 나라를 사전순으로 출력
SELECT country
FROM User
WHERE
    name not like 'J%'
ORDER BY country;
