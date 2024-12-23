-- Active: 1727165140235@@127.0.0.1@3306@maple_database
-- 가장 레벨이 높은 캐릭터를 키우는 유저의 캐릭터 중 가장 레벨이 낮은 캐릭터의 레벨을 출력하세요.
-- SELECT LEVEL FROM (SELECT owner_id, level FROM raisingcharacter WHERE level=(SELECT owner_id, level FROM ))

WITH
    char_level AS (
        SELECT level
        FROM raisingcharacter
        WHERE
            owner_id = (
                SELECT owner_id
                FROM raisingcharacter
                WHERE
                    level = (
                        SELECT max(level)
                        FROM raisingcharacter
                    )
            )
    )
SELECT level
FROM char_level
WHERE
    level = (
        SELECT min(level)
        FROM char_level
    )
