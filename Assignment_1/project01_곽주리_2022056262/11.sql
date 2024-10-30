-- Active: 1727165140235@@127.0.0.1@3306@maple_database
-- 닉네임이 j로 시작하는 캐릭터를 키우고 있는 유저의 이름과 캐릭터 닉네임을 유저 이름의 사전순으로 출력
SELECT name, nickname
FROM User, (
        SELECT owner_id, nickname
        FROM raisingcharacter
        where
            nickname like 'j%'
    ) AS j_character
WHERE
    id = owner_id
ORDER BY name;
