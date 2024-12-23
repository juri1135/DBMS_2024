-- Active: 1727165140235@@127.0.0.1@3306@maple_database
-- USA 출신 유저 중 키우고 있는 캐릭터 레벨의 평균이 가장 높은 유저의 이름을 출력

WITH
    usaUser_level AS (
        SELECT name, avg(level) as avg_level
        FROM (
                SELECT name, id
                FROM user
                WHERE
                    country = 'USA'
            ) AS usa_user
            JOIN raisingcharacter AS USA_user_char ON usa_user.id = owner_id
        GROUP BY
            owner_id
    )
SELECT name
FROM usaUser_level
WHERE
    avg_level = (
        SELECT max(avg_level)
        FROM usaUser_level
    );
