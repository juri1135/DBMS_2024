--Asia에 위치한 나라의 유튜버의 유저 이름을 출력
SELECT name
FROM User, (
        SELECT youtuber_id
        FROM youtuber
        where
            country in (
                SELECT name
                FROM country
                WHERE
                    continent = 'Asia'
            )
    ) AS Asia_youtuber
WHERE
    id = youtuber_id;
