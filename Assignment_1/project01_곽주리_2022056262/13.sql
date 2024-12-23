-- Active: 1727165140235@@127.0.0.1@3306@maple_database
-- Warrior branch�� ĳ���͸� ���� ���� Ű��� ���� ������ id�� �� ������ Warrior branch ĳ������ �� ���
--playable���� branch�� warrior�� relation�� raisingchar�� natural join�Ѵ�.
-- join on ���� Ȥ�� �� ���� ���� ����!! (lab ppt ����) ������ cid�� playable�� id ��ġ ����
-- ���⼭ owner_id�� ������ ��.
-- count�� �׳� �ϸ� warrior Ű��� user ���� ���ͼ� user_id�� grouping���ְ�
-- count �Լ� �Ἥ �� group�� �� ������ �˾Ƴ��� ��
-- �˾Ƴ��� ������������ �����ϰ� ù ��° tuple�� ���� ���� ��
-- ù ��° tuple ���� ���� ��... �˻��ؼ� ã��;; LIMIT ����ϸ� ��

--���� db�� data�� ����ȴٸ�, LIMIT ���� ���� data�� ���� ���� �� �Ѵٴ� ������ ����...
--with as ����ؼ� id�� ĳ���� ���� �����ϰ� �ű⼭ owner_id_count�� max���� ������ data�� �� ����ϱ�...
WITH
    count_char AS (
        SELECT owner_id, COUNT(owner_id) AS owner_id_count
        FROM raisingcharacter
            JOIN (
                SELECT id
                FROM playablecharacter
                WHERE
                    branch = 'Warrior'
            ) AS owner ON raisingcharacter.cid = owner.id
        GROUP BY
            owner_id
        ORDER BY owner_id_count DESC
    )
SELECT owner_id, owner_id_count
FROM count_char
WHERE
    owner_id_count = (
        SELECT MAX(owner_id_count)
        FROM count_char
    );
