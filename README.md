# ATSD ODBC Driver

## Overview

The driver provides access to the Axibase Time Series Database for applications that support the ODBC database connectivity protocol. It is primarily used by programs running on Microsoft Windows.

## Syntax

ATSD SQL supports the following DML query types.

* `SELECT`
* `INSERT`
* `UPDATE`
* `DELETE`

The syntax and examples are provided in the [SQL](https://axibase.com/docs/atsd/sql/) documentation.

DDL statements are not supported since the ATSD schema is self-managed.

## Protocol

The driver communicates with the ATSD using the SSL protocol. Enable the **Ignore SSL Errors** setting in case the target server is using the self-signed [certificate](https://axibase.com/docs/atsd/administration/ssl-ca-signed.html).

## Installation

* Download the latest `msi` package for your platform from the [Releases](https://github.com/axibase/atsd-odbc/releases) section.
* Make sure that the package file (32 or 64 bit) corresponds to the OS platform.
* Follow the prompts to install the driver.

## Configuration

* Open **Administrative Tools**, click **Data Sources (ODBC)**.
* Select driver encoding.
* Click **Add** on the **User DSN** tab and specify connection settings.

Property | Required | Description
---|---|---
Name | yes | Data source name
Description | no | Short data source description
Host | yes | Hostname or IP address of the target ATSD server
Port | yes | HTTPS Port of the target ATSD server. Default is 8443.
Ignore SSL Errors | yes | Ignore SSL certificate validation errors.
User | yes | Username
Password | yes | Password
Table | no | Expression to filter available tables (metrics). Use `%` as a wildcard.
Expand Tags | No | Expand the `tags` column into multiple columns.
Meta Columns | No | Add metadata columns in `SELECT *` columns and table metadata queries.
