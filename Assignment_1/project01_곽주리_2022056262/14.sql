-- Active: 1727165140235@@127.0.0.1@3306@maple_database
-- 캐릭터 ID가 한 자리 수인 캐릭터의 branch를 캐릭터 id의 내림차순으로 출력
SELECT branch
FROM playablecharacter
WHERE
    id like '_'
ORDER BY id desc;
