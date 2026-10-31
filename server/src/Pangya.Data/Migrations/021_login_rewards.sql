-- Prêmio de login por evento (config GameJp.LoginRewards): progresso de cada conta em cada evento.
create table login_rewards (
    account_id bigint not null references accounts(id) on delete cascade,
    event_id   integer not null,
    days       integer not null,
    times      integer not null,
    clear      boolean not null,
    last_day   date not null,
    primary key (account_id, event_id)
);
