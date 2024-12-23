-- Active: 1727165140235@@127.0.0.1@3306@maple_database
-- 모든 유저가 키우는 중인 Cygnus Knight class 캐릭터의 평균 레벨
SELECT avg(level)
FROM
    raisingcharacter
    JOIN (
        SELECT id
        FROM playablecharacter
        WHERE
            class = 'Cygnus_knights'
    ) as cygnus ON cid = cygnus.id;
