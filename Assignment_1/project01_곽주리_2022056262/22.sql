-- Active: 1727165140235@@127.0.0.1@3306@maple_database
-- 이름이 n으로 시작하는 캐릭터를 키우는 유저 중 Korea 출신 유저의 이름을 사전순으로 모두 출력
SELECT name
FROM User
    JOIN (
        SELECT owner_id
        FROM (
                SELECT id
                FROM playablecharacter
                WHERE
                    name like 'N%'
            ) AS N_id
            JOIN raisingcharacter AS N_char ON cid = N_id.id
    ) AS N_user ON user.id = owner_id
WHERE
    country = 'Korea'
ORDER BY name;
