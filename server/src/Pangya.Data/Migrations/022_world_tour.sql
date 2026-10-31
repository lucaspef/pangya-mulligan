-- World Tour (JP): um bit por campo completado (18 buracos) e se o prêmio já foi pego.
create table world_tour (
    account_id bigint primary key references accounts(id) on delete cascade,
    map_flag   integer not null,
    got_gift   boolean not null
);
