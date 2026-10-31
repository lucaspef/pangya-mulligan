-- Saldo da Point Shop antiga (Legacy Tiki) do cliente JP 983: não é item, é um contador do jogador.
create table tiki_points (
    account_id bigint primary key references accounts(id) on delete cascade,
    points     integer not null default 0
);
