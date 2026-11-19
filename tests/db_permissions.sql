DO $$
BEGIN
    IF (SELECT rolsuper OR rolcreatedb OR rolcreaterole OR rolbypassrls FROM pg_roles WHERE rolname = 'nobody')
       OR has_schema_privilege('nobody', 'public', 'CREATE')
       OR has_column_privilege('nobody', 'accounts', 'password_hash', 'UPDATE')
       OR has_table_privilege('nobody', 'accounts', 'DELETE')
       OR has_table_privilege('nobody', 'results', 'TRUNCATE')
       OR has_table_privilege('nobody', 'results', 'UPDATE')
       OR has_column_privilege('nobody', 'users', 'email', 'SELECT') THEN
        RAISE EXCEPTION 'Unexpected database privileges for the API role';
    END IF;
END
$$;
