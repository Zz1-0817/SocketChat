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

-- 2. chat_rooms 表（id 为 INTEGER）
CREATE TABLE IF NOT EXISTS chat_rooms (
    id   INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT
);

-- 3. room_members 表（room_id 为 INTEGER）
CREATE TABLE IF NOT EXISTS room_members (
    room_id INTEGER NOT NULL,
    user_id INTEGER NOT NULL,
    PRIMARY KEY (room_id, user_id)
);

-- 4. messages 表（room_id 为 INTEGER）
CREATE TABLE IF NOT EXISTS messages (
    id        INTEGER PRIMARY KEY AUTOINCREMENT,
    sender    INTEGER NOT NULL,     -- 用户 ID
    receiver  INTEGER DEFAULT NULL, -- 私聊时的对方用户 ID
    room_id   INTEGER DEFAULT NULL, -- 群聊房间 ID
    timestamp INTEGER NOT NULL,     -- UNIX 时间戳
    content   TEXT    NOT NULL,
    type      INTEGER NOT NULL      -- 0: 私聊, 1: 群聊, 2: 系统通知
);

-- 插入示例用户（id 自动生成为 1, 2, 3）
INSERT INTO users (username, password, nickname, status) VALUES
  ('alice', 'alice_pwd', 'Alice', 1),
  ('bob',   'bob_pwd',   'Bob',   1),
  ('carol', 'carol_pwd', 'Carol', 0);

-- 插入示例聊天室（id 自动生成为 1, 2）
INSERT INTO chat_rooms (name) VALUES
  ('General Chat'),
  ('Project Room');

-- 插入示例房间成员
INSERT INTO room_members (room_id, user_id) VALUES
  (1, 1),
  (1, 2),
  (2, 2),
  (2, 3);

-- 插入示例消息（使用整数 ID）
INSERT INTO messages (sender, receiver, room_id, timestamp, content, type) VALUES
  (1, 2, NULL, strftime('%s','now') - 3600, 'Hey Bob!',      0),
  (2, 1, NULL, strftime('%s','now') - 3500, 'Hi Alice!',     0),
  (2, NULL, 1, strftime('%s','now') - 1800, 'Welcome folks', 1),
  (3, NULL, 2, strftime('%s','now') - 1200, 'Kickoff at 10', 1),
  (0, 1, NULL, strftime('%s','now'),        'Login OK',      2); -- 系统通知 sender = 0

