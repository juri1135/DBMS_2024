-- Active: 1727165140235@@127.0.0.1@3306@maple_database
--�̸��� Luminus�� ĳ���Ϳ� ���� branch�� ���� ĳ������ �̸��� ������ ���
SELECT name
FROM playablecharacter
WHERE
    branch = (
        SELECT branch
        FROM playablecharacter
        WHERE
            name = 'Luminus'
    )
ORDER BY name;
