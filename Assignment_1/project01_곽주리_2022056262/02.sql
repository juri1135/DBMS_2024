-- Active: 1727165140235@@127.0.0.1@3306@maple_database
--UK �Ǵ� Australia ��� user�� �̸��� ���� ������ ����ϼ���--
SELECT *
FROM (
        SELECT name
        FROM User
        WHERE
            country = 'UK'
        UNION
        SELECT name
        FROM User
        WHERE
            country = 'Australia'
    )
    -- from���� subquery�� ������ ��Ī�� �ʿ��ϴ�.
    -- from���� subquery�� ������ �ӽ� relation�� ����� �ǵ�,
    -- subquery ������ �Ӽ��� �� ���������� �̸����� ���ؾ�
    -- schema�� �ϼ��Ǵ� ��
    AS combined_names
ORDER BY name;
