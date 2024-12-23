-- Active: 1727165140235@@127.0.0.1@3306@maple_database
-- ���� ������ ���� ĳ���͸� Ű��� ������ ĳ���� �� ���� ������ ���� ĳ������ ������ ����ϼ���.
-- SELECT LEVEL FROM (SELECT owner_id, level FROM raisingcharacter WHERE level=(SELECT owner_id, level FROM ))

WITH
    char_level AS (
        SELECT level
        FROM raisingcharacter
        WHERE
            owner_id = (
                SELECT owner_id
                FROM raisingcharacter
                WHERE
                    level = (
                        SELECT max(level)
                        FROM raisingcharacter
                    )
            )
    )
SELECT level
FROM char_level
WHERE
    level = (
        SELECT min(level)
        FROM char_level
    )
