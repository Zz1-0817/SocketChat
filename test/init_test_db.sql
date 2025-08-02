-- init_test_db.sql

-- 清理旧表（如果存在）
DROP TABLE IF EXISTS room_members;
DROP TABLE IF EXISTS messages;
DROP TABLE IF EXISTS chat_rooms;
DROP TABLE IF EXISTS users;

-- 1. users 表
CREATE TABLE IF NOT EXISTS users (
    id        INTEGER PRIMARY KEY AUTOINCREMENT,
    username  TEXT    UNIQUE NOT NULL,
    password  TEXT,
    nickname  TEXT,
    status    INTEGER DEFAULT 0
);

-- 2. chat_rooms 表
CREATE TABLE IF NOT EXISTS chat_rooms (
    id   TEXT PRIMARY KEY,
    name TEXT
);

-- 3. room_members 表
CREATE TABLE IF NOT EXISTS room_members (
    room_id TEXT NOT NULL,
    user_id INTEGER NOT NULL,
    PRIMARY KEY (room_id, user_id)
);

-- 4. messages 表
CREATE TABLE IF NOT EXISTS messages (
    id        INTEGER PRIMARY KEY AUTOINCREMENT,
    sender    TEXT    NOT NULL,     -- 私聊/群聊/通知的发送者
    receiver  TEXT    DEFAULT NULL, -- 私聊时的对方用户名
    room_id   TEXT    DEFAULT NULL, -- 群聊房间 ID
    timestamp INTEGER NOT NULL,     -- UNIX 时间戳
    content   TEXT    NOT NULL,
    type      INTEGER NOT NULL      -- 0: 私聊, 1: 群聊, 2: 系统通知
);

-- 插入示例用户
INSERT INTO users (username, password, nickname, status) VALUES
  ('alice', 'alice_pwd', 'Alice', 1),
  ('bob',   'bob_pwd',   'Bob',   1),
  ('carol', 'carol_pwd', 'Carol', 0);

-- 插入示例聊天室
INSERT INTO chat_rooms (id, name) VALUES
  ('room1', 'General Chat'),
  ('room2', 'Project Room');

-- 插入示例房间成员
INSERT INTO room_members (room_id, user_id) VALUES
  ('room1', 1),
  ('room1', 2),
  ('room2', 2),
  ('room2', 3);

-- 插入示例消息
INSERT INTO messages (sender, receiver, room_id, timestamp, content, type) VALUES
  ('alice',   'bob',   NULL,       strftime('%s','now') - 3600, 'Hey Bob!',      0),
  ('bob',     'alice', NULL,       strftime('%s','now') - 3500, 'Hi Alice!',     0),
  ('bob',     NULL,    'room1',    strftime('%s','now') - 1800, 'Welcome folks', 1),
  ('carol',   NULL,    'room2',    strftime('%s','now') - 1200, 'Kickoff at 10', 1),
  ('system',  'alice', NULL,       strftime('%s','now'),          'Login OK',      2);

