--Asia�� ��ġ�� ������ ��Ʃ���� ���� �̸��� ���
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
