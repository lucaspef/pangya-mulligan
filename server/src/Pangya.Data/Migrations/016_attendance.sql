-- Presença diária (cliente JP 983): último dia em que entrou, dias contados e o próximo prêmio (mostrado ao sair).
create table attendance (
    account_id bigint primary key references accounts(id) on delete cascade,
    last_day   date not null,
    days       integer not null,
    next_tid   integer not null,
    next_qty   integer not null
);
