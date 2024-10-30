-- Active: 1727165140235@@127.0.0.1@3306@maple_database
--UK 또는 Australia 출신 user의 이름을 사전 순으로 출력하세요--
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
    -- from절의 subquery는 무조건 별칭이 필요하다.
    -- from절의 subquery는 가상의 임시 relation을 만드는 건데,
    -- subquery 내에서 속성은 다 정의했으니 이름까지 정해야
    -- schema가 완성되는 것
    AS combined_names
ORDER BY name;
