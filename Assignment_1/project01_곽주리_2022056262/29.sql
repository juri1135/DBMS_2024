-- Active: 1727165140235@@127.0.0.1@3306@maple_database
-- Resistance class 캐릭터 전부와 겹치지 않는 branch를 가지는 캐릭터를 가장 많이 키우는 유저의 이름 출력

WITH
    thief_user AS (
        SELECT owner_id, COUNT(owner_id) AS owner_id_count
        FROM (
                SELECT id, branch
                FROM playablecharacter
                WHERE
                    branch NOT IN(
                        SELECT branch
                        FROM playablecharacter
                        WHERE
                            class = 'Resistance'
                    )
            ) AS thief
            JOIN raisingcharacter ON thief.id = raisingcharacter.cid
        GROUP BY
            owner_id
    )
SELECT user_name.name
FROM thief_user
    JOIN user AS user_name ON thief_user.owner_id = user_name.id
WHERE
    thief_user.owner_id_count = (
        SELECT MAX(owner_id_count)
        FROM thief_user
    );
