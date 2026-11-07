/**
 * Backend do monitor de temperatura e umidade
 *
 * Recebe as leituras enviadas pela placa ESP8266 (Wemos) por HTTP POST
 * em formato JSON e grava cada uma em um banco SQLite (arquivo leituras.db).
 * Também oferece uma rota GET para consultar as leituras gravadas.
 */

const express = require("express");
const Database = require("better-sqlite3");
const path = require("path");

const app = express();
const PORTA = 3000;

// Permite que o Express leia o corpo das requisições em JSON
app.use(express.json());

// Abre (ou cria, se não existir) o arquivo do banco de dados
// na mesma pasta do server.js
const db = new Database(path.join(__dirname, "leituras.db"));

// Cria a tabela de leituras na primeira execução
db.exec(`
  CREATE TABLE IF NOT EXISTS leituras (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    dispositivo TEXT NOT NULL,
    temperatura REAL NOT NULL,
    umidade REAL NOT NULL,
    criado_em DATETIME DEFAULT CURRENT_TIMESTAMP
  )
`);

// Comando de inserção preparado uma vez e reaproveitado a cada leitura
const inserir = db.prepare(
  "INSERT INTO leituras (dispositivo, temperatura, umidade) VALUES (?, ?, ?)"
);

// POST /leituras: recebe uma leitura da placa
// Corpo esperado: {"dispositivo":"wemos-1","temperatura":25,"umidade":60}
app.post("/leituras", (req, res) => {
  const { dispositivo, temperatura, umidade } = req.body;

  // Valida os dados antes de gravar
  if (
    !dispositivo ||
    typeof temperatura !== "number" ||
    typeof umidade !== "number"
  ) {
    console.log("Requisição inválida:", req.body);
    return res.status(400).json({ erro: "Dados inválidos" });
  }

  // Grava no banco e mostra no terminal do servidor
  inserir.run(dispositivo, temperatura, umidade);
  console.log(
    `Leitura gravada: ${dispositivo} | ${temperatura} °C | ${umidade} %`
  );

  // 201 = criado com sucesso (é esse código que a placa mostra na serial)
  res.status(201).json({ ok: true });
});

// GET /leituras: devolve as 50 leituras mais recentes
app.get("/leituras", (req, res) => {
  const linhas = db
    .prepare("SELECT * FROM leituras ORDER BY id DESC LIMIT 50")
    .all();
  res.json(linhas);
});

// "0.0.0.0" faz o servidor aceitar conexões de outros dispositivos da rede
// (sem isso, a Wemos não conseguiria acessar o PC)
app.listen(PORTA, "0.0.0.0", () => {
  console.log(`Servidor rodando na porta ${PORTA}`);
});