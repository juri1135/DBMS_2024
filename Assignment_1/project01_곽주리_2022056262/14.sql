-- Active: 1727165140235@@127.0.0.1@3306@maple_database
-- ĳ���� ID�� �� �ڸ� ���� ĳ������ branch�� ĳ���� id�� ������������ ���
SELECT branch
FROM playablecharacter
WHERE
    id like '_'
ORDER BY id desc;
