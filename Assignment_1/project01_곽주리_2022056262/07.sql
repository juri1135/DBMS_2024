-- Active: 1727165140235@@127.0.0.1@3306@maple_database
-- ĳ���� �̸��� A,B,C,D�� �����ϴ� ĳ������ �̸��� ���������� ����ϼ���
SELECT name
FROM playablecharacter
WHERE
    name like 'A%'
    OR name like 'B%'
    OR name like 'C%'
    OR name like 'D%'
ORDER BY name; 
