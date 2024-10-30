-- Active: 1727165140235@@127.0.0.1@3306@maple_database
-- 유튜버들의 이름과 각 유튜버들이 키우고 있는 캐릭터들의 레벨 합을 레벨 합의 내림차순으로 출력

SELECT name, sum(level)
FROM (
        SELECT owner_id, level
        FROM raisingcharacter
    ) AS youtuber
    JOIN (
        SELECT id, name
        FROM
            youtuber
            JOIN User AS youtuber_info ON youtuber_id = id
    ) AS youtuber_char ON id = owner_id
GROUP BY
    name
ORDER BY sum(level) desc;
