-- Active: 1727165140235@@127.0.0.1@3306@maple_database
-- Nova �Ǵ� RESISTANCE CLASS�̸鼭 WARRIOR BRANCH�� ĳ���� ������
SELECT name
FROM playablecharacter
WHERE (
        class = 'Nova'
        OR class = 'Resistance'
    )
    AND branch = 'Warrior'
ORDER BY name;
