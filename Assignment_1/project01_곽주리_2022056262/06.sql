-- Active: 1727165140235@@127.0.0.1@3306@maple_database
-- Raising Character �� ���� ������ ���� ĳ������ �г����� ����ϼ���
SELECT nickname
FROM raisingcharacter
    -- where������ tuple���� ���ϱ� ������ max�� subquery �ȿ� �־�� �Ѵ�.
    -- aggregate functions�� ����� single value�̱� ����. 
    -- subquery�� ��ȯ���� relation�̶� tuple���� �񱳰� ����
    -- �̷� �� scalar subquery 
where
    level = (
        SELECT max(level)
        FROM raisingcharacter
    );
