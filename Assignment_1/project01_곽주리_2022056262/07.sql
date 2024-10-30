-- Active: 1727165140235@@127.0.0.1@3306@maple_database
-- 캐릭터 이름이 A,B,C,D로 시작하는 캐릭터의 이름을 사전순으로 출력하세요
SELECT name
FROM playablecharacter
WHERE
    name like 'A%'
    OR name like 'B%'
    OR name like 'C%'
    OR name like 'D%'
ORDER BY name; 
