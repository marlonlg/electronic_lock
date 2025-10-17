
import sqlite3
import os
from typing import Dict, Any, Optional, List

import sqlite3
from typing import Dict, Any, Optional, List




class UserRecords:
    def __init__(self, db_name: str = "records.db"):
        self.db_name = db_name
        if not os.path.exists(self.db_name):
            self.create_database(self.db_name)
        self._connect()

    @staticmethod
    def create_database(db_name="records.db"):
        """
        Cria o banco de dados e a tabela 'records' se não existir.
        """
        conn = sqlite3.connect(db_name)
        cursor = conn.cursor()
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS records (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                usuario TEXT NOT NULL UNIQUE,
                senha TEXT NOT NULL,
                tag_rfid INTEGER UNIQUE
            )
        ''')
        conn.commit()
        conn.close()
        print(f"Banco de dados criado ou já existente: '{db_name}'")

    def _connect(self):
        """Conecta ao banco e configura row_factory para retornar dicionários."""
        self.conn = sqlite3.connect(self.db_name)
        self.conn.row_factory = sqlite3.Row  # Permite acesso por nome de coluna
        self.cursor = self.conn.cursor()

    def _execute_query(self, query: str, params: tuple = ()) -> Optional[sqlite3.Cursor]:
        """Executa uma query genérica com tratamento de erro e commit automático."""
        try:
            cursor = self.cursor.execute(query, params)
            self.conn.commit()
            return cursor
        except sqlite3.IntegrityError as e:
            print(f"Erro de integridade: {e}")
            self.conn.rollback()
            raise
        except Exception as e:
            print(f"Erro inesperado: {e}")
            self.conn.rollback()
            raise

    def insert(self, data:Dict[str, Any]) -> int:
        """
        Insere um novo usuário usando JSON.
        Exemplo: {"usuario": "joao", "senha": "123", "tag_rfid": 98765}
        Retorna o ID inserido.
        """
        try:
            required_fields = {"usuario", "senha", "tag_rfid"}
            if not required_fields.issubset(data.keys()):
                raise ValueError(f"Campos obrigatórios faltando: {required_fields - set(data.keys())}")

            query = "INSERT INTO records (usuario, senha, tag_rfid) VALUES (?, ?, ?)"
            self._execute_query(query, (data["usuario"], data["senha"], data["tag_rfid"]))
            return self.cursor.lastrowid
        except Exception as e:
            print(e)
            return None

    def update(self, user_id: int, data: Dict[str, Any]) -> bool:
        """
        Atualiza um usuário pelo ID usando JSON.
        Exemplo: {"usuario": "joao_novo", "senha": "nova_senha"}
        """
        if not data: 
            raise ValueError("Nenhum dado fornecido para atualização.")

        # Constrói dinamicamente a query
        set_clause = ", ".join(f"{key} = ?" for key in data.keys())
        query = f"UPDATE records SET {set_clause} WHERE id = ?"
        params = tuple(data.values()) + (user_id,)

        self._execute_query(query, params)
        return self.cursor.rowcount > 0

    def delete(self, user_id: int) -> bool:
        
        """Deleta um usuário pelo ID."""
        query = "DELETE FROM records WHERE id = ?"
        self._execute_query(query, (user_id,))
        return self.cursor.rowcount > 0

    def get_by_id(self, user_id: int) -> Optional[Dict[str, Any]]:
        """Busca um usuário pelo ID e retorna como dicionário."""
        query = "SELECT * FROM records WHERE id = ?"
        self.cursor.execute(query, (user_id,))
        row = self.cursor.fetchone()
        return dict(row) if row else None

    def get_all(self) -> List[Dict[str, Any]]:
        """Retorna todos os usuários como lista de dicionários."""
        query = "SELECT * FROM records"
        self.cursor.execute(query)
        return [dict(row) for row in self.cursor.fetchall()]

    def close(self):
        """Fecha a conexão."""
        if self.conn:
            self.conn.close()
