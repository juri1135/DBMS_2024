-- Active: 1727165140235@@127.0.0.1@3306@maple_database
-- �г����� j�� �����ϴ� ĳ���͸� Ű��� �ִ� ������ �̸��� ĳ���� �г����� ���� �̸��� ���������� ���
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
