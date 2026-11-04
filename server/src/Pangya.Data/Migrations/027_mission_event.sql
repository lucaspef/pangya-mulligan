-- Mission do lobby (JP): contadores do dia (com a data) e do período, e os prêmios já pegos.
create table mission_event (
    account_id  bigint primary key references accounts(id) on delete cascade,
    day         date not null,
    day_versus  integer not null,
    day_tourney integer not null,
    day_gift    boolean not null,
    versus      integer not null,
    long_putt   integer not null,
    record      integer not null,
    eagles      integer not null,
    courses     integer not null,
    term_gift   boolean not null
);
