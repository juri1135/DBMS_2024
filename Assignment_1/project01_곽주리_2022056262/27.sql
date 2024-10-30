-- Active: 1727165140235@@127.0.0.1@3306@maple_database
-- Nova 또는 RESISTANCE CLASS이면서 WARRIOR BRANCH인 캐릭터 사전순
SELECT name
FROM playablecharacter
WHERE (
        class = 'Nova'
        OR class = 'Resistance'
    )
    AND branch = 'Warrior'
ORDER BY name;
