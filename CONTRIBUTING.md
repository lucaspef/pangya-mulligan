# Contribuindo

Bugs, correções, documentação e novidades são bem-vindos. Abra uma issue ou um pull request.

## Regras

- **Nunca envie arquivos do jogo:** `.exe`, `.dll`, `.pak`, `.iff`, `.pet`, imagens, sons ou dados extraídos. Também não envie SDKs, dumps, logs de pacotes, senhas nem dados de jogadores.
- Ao enviar uma contribuição, você a licencia sob a AGPL-3.0 e declara ter o direito de fazê-lo. Código de terceiros precisa de licença compatível com a AGPL-3.0 e do aviso correspondente em `NOTICE.md`.
- Nomes de código em inglês; comentários em português ou inglês.
- Não renomeie os macros e identificadores `JP983`: eles identificam a versão do cliente.
- Toda alteração do servidor precisa de teste.

## Como testar

- **Cliente:** veja `client/COMPILAR.md`. Compile Release e Debug, Win32.
- **Servidor** (WSL ou Linux, .NET 10 e PostgreSQL), em `server/`:

  ```bash
  bash verify.sh
  ```

  `tools/setup-dev.sh`, chamado pelo `verify.sh`, cria os bancos e as configurações locais; rode só em ambiente de desenvolvimento. Os testes marcados em `tests/needs-game-data.txt` precisam de `data/pangya.iff` e `data/pangya_jp.iff` da sua própria cópia do jogo, e são omitidos sem esses arquivos. Os testes de protocolo exigem `EMU` e também são omitidos sem essa variável.
