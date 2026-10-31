-- Pontos do Grand Zodiac por conta (cliente JP 983): somados nos eventos; definem o tamanho do copo (0x1F9).
create table zodiac_points (
    account_id bigint primary key references accounts(id) on delete cascade,
    points     integer not null default 0
);
