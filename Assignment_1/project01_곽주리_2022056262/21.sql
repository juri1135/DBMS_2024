-- Active: 1727165140235@@127.0.0.1@3306@maple_database
-- korea 출신 유저와 uk 출신 유저가 공통으로 키우고 있는 캐릭터의 이름을 사전순으로 출력

SELECT name
FROM (
        SELECT DISTINCT
            cid
        FROM raisingcharacter
            JOIN (
                SELECT id
                FROM user
                WHERE
                    country = 'Korea'
            ) AS korea_char
        WHERE
            korea_char.id = owner_id
            AND cid in (
                SELECT cid
                FROM raisingcharacter
                    JOIN (
                        SELECT id
                        FROM user
                        WHERE
                            country = 'UK'
                    ) AS uk_char
                WHERE
                    uk_char.id = owner_id
            )
    ) AS uk_and_ko
    JOIN playablecharacter AS uk_ko_char ON cid = id
ORDER BY name;
