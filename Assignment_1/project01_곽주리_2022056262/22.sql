-- Active: 1727165140235@@127.0.0.1@3306@maple_database
-- �̸��� n���� �����ϴ� ĳ���͸� Ű��� ���� �� Korea ��� ������ �̸��� ���������� ��� ���
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
