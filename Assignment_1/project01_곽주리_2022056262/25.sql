-- Active: 1727165140235@@127.0.0.1@3306@maple_database
-- 어느 유저도 키우지 않는 캐릭터의 이름을 사전 순으로 출력
SELECT name
FROM playablecharacter
WHERE
    id not in(
        SELECT cid
        FROM raisingcharacter
    )
ORDER BY name;
