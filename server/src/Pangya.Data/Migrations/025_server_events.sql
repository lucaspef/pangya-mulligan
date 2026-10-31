-- Campos de evento do sGameServerInfo (lista de servidores do JP 983); bits crus: u32 gravado em int.
alter table servers
    add column event_flags int not null default 0,
    add column event_value int not null default 0,
    add column icon_index  int not null default 0;
