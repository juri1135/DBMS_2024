-- Active: 1727165140235@@127.0.0.1@3306@maple_database
-- Chang이 키우고 있는 캐릭터 중 가장 높은 레벨의 캐릭터의 레벨을 출력하세요
SELECT max(level)
FROM (
        SELECT level
        FROM user, raisingcharacter
        where
            user.id = raisingcharacter.owner_id
    ) AS chang_character;
