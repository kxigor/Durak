BEGIN;
DO $$
BEGIN
    IF NOT EXISTS (SELECT FROM pg_roles WHERE rolname = 'nobody') THEN
        CREATE ROLE nobody LOGIN;
    END IF;
END
$$;
ALTER ROLE postgres PASSWORD NULL;
REVOKE ALL ON DATABASE durak FROM PUBLIC;
GRANT CONNECT ON DATABASE durak TO nobody;
REVOKE ALL ON SCHEMA public FROM PUBLIC;
GRANT USAGE ON SCHEMA public TO nobody;

CREATE TABLE IF NOT EXISTS users (
    login text PRIMARY KEY,
    email text NOT NULL
);
CREATE TABLE IF NOT EXISTS results (
    id text PRIMARY KEY,
    name text NOT NULL,
    outcome text NOT NULL CHECK (outcome IN ('win', 'loss', 'draw'))
);
CREATE TABLE IF NOT EXISTS accounts (
    login text PRIMARY KEY,
    password_hash text NOT NULL,
    session_digest text UNIQUE,
    expires_at timestamptz
);
ALTER TABLE results ADD COLUMN IF NOT EXISTS owner text REFERENCES accounts(login);
GRANT SELECT, INSERT ON accounts TO nobody;
GRANT UPDATE (session_digest, expires_at) ON accounts TO nobody;
CREATE INDEX IF NOT EXISTS results_name ON results (name);
DELETE FROM users
WHERE (login, email) IN (('pavel', 'pavel@example.invalid'),
                         ('yura', 'yura@example.invalid'));
INSERT INTO users (login, email)
VALUES ('Igor', 'igor@example.invalid'), ('Pavel', 'pavel@example.invalid')
ON CONFLICT (login) DO NOTHING;
GRANT SELECT (login) ON users TO nobody;
GRANT SELECT, INSERT ON results TO nobody;
COMMIT;
