-- Active: 1727165140235@@127.0.0.1@3306@maple_database
-- 유저의 이름과 그 유저가 키우는 캐릭터 이름을 출력. 유저 이름은 사전 순, 유저가 키우는 캐릭터 사전 순 정렬
SELECT user_char.name, x.name char_name
FROM (
        SELECT owner_id, name
        FROM
            playablecharacter
            JOIN raisingcharacter AS char_name ON cid = playablecharacter.id
        ORDER BY name
    ) AS x
    JOIN User AS user_char ON id = owner_id
ORDER BY user_char.name, x.name;
