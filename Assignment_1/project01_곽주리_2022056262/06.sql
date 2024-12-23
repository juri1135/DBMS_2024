-- Active: 1727165140235@@127.0.0.1@3306@maple_database
-- Raising Character 중 가장 레벨이 높은 캐릭터의 닉네임을 출력하세요
SELECT nickname
FROM raisingcharacter
    -- where에서는 tuple끼리 비교하기 때문에 max를 subquery 안에 넣어야 한다.
    -- aggregate functions의 결과는 single value이기 때문. 
    -- subquery의 반환형은 relation이라서 tuple끼리 비교가 가능
    -- 이런 게 scalar subquery 
where
    level = (
        SELECT max(level)
        FROM raisingcharacter
    );
