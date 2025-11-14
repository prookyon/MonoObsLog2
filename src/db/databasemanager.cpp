#include "db/databasemanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QFile>

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent), m_initialized(false)
{
}

DatabaseManager::~DatabaseManager()
{
    if (m_database.isOpen())
    {
        m_database.close();
    }
}

bool DatabaseManager::initialize(const QString &dbPath)
{
    m_dbPath = dbPath;

    // Check if database file exists
    bool dbExists = QFile::exists(m_dbPath);

    // Open/create the database
    m_database = QSqlDatabase::addDatabase("QSQLITE");
    m_database.setDatabaseName(m_dbPath);

    if (!m_database.open())
    {
        emit errorOccurred(QString("Failed to open database: %1").arg(m_database.lastError().text()));
        return false;
    }

    // If database didn't exist, create tables
    if (!dbExists)
    {
        if (!createTables())
        {
            emit errorOccurred("Failed to create database tables");
            return false;
        }
    }

    m_initialized = true;
    emit databaseInitialized();
    return true;
}

bool DatabaseManager::isOpen() const
{
    return m_database.isOpen();
}

QSqlDatabase &DatabaseManager::database()
{
    return m_database;
}

bool DatabaseManager::createTables()
{
    QSqlQuery query(m_database);

    // Create filter_types table first (referenced by filters)
    QString sql = R"(
        CREATE TABLE filter_types (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL UNIQUE,
            priority INTEGER DEFAULT 0
        )
    )";

    if (!query.exec(sql))
    {
        qDebug() << "Failed to create filter_types table:" << query.lastError().text();
        return false;
    }

    // Create cameras table
    sql = R"(
        CREATE TABLE cameras (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL UNIQUE,
            sensor TEXT NOT NULL,
            pixel_size REAL NOT NULL,
            width INTEGER NOT NULL,
            height INTEGER NOT NULL
        )
    )";

    if (!query.exec(sql))
    {
        qDebug() << "Failed to create cameras table:" << query.lastError().text();
        return false;
    }

    // Create telescopes table
    sql = R"(
        CREATE TABLE telescopes (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL UNIQUE,
            aperture INTEGER NOT NULL,
            f_ratio REAL NOT NULL,
            focal_length INTEGER NOT NULL
        )
    )";

    if (!query.exec(sql))
    {
        qDebug() << "Failed to create telescopes table:" << query.lastError().text();
        return false;
    }

    // Create objects table
    sql = R"(
        CREATE TABLE objects (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL UNIQUE,
            ra REAL,
            dec REAL
        )
    )";

    if (!query.exec(sql))
    {
        qDebug() << "Failed to create objects table:" << query.lastError().text();
        return false;
    }

    // Create sessions table
    sql = R"(
        CREATE TABLE sessions (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL UNIQUE,
            start_date TEXT NOT NULL,
            moon_illumination REAL,
            moon_ra REAL,
            moon_dec REAL,
            comments TEXT
        )
    )";

    if (!query.exec(sql))
    {
        qDebug() << "Failed to create sessions table:" << query.lastError().text();
        return false;
    }

    // Create filters table (depends on filter_types)
    sql = R"(
        CREATE TABLE filters (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL UNIQUE,
            filter_type_id INTEGER NOT NULL,
            FOREIGN KEY(filter_type_id) REFERENCES filter_types(id)
        )
    )";

    if (!query.exec(sql))
    {
        qDebug() << "Failed to create filters table:" << query.lastError().text();
        return false;
    }

    // Create observations table (depends on multiple tables)
    sql = R"(
        CREATE TABLE observations (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            image_count INTEGER NOT NULL,
            exposure_length INTEGER NOT NULL,
            total_exposure INTEGER NOT NULL,
            comments TEXT,
            session_id INTEGER NOT NULL,
            object_id INTEGER NOT NULL,
            camera_id INTEGER NOT NULL,
            telescope_id INTEGER NOT NULL,
            filter_id INTEGER NOT NULL,
            FOREIGN KEY(session_id) REFERENCES sessions(id),
            FOREIGN KEY(object_id) REFERENCES objects(id),
            FOREIGN KEY(camera_id) REFERENCES cameras(id),
            FOREIGN KEY(telescope_id) REFERENCES telescopes(id),
            FOREIGN KEY(filter_id) REFERENCES filters(id)
        )
    )";

    if (!query.exec(sql))
    {
        qDebug() << "Failed to create observations table:" << query.lastError().text();
        return false;
    }

    qDebug() << "All database tables created successfully";
    return true;
}

bool DatabaseManager::tableExists(const QString &tableName)
{
    return m_database.tables().contains(tableName);
}