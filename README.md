# desktop-order

Harmoni POS Desktop is the Qt Widgets counter checkout client for the Harmoni POS platform. It is a native C++ desktop app that talks to the gateway over HTTP using absolute endpoint URLs from `config.app.ini`; it does not connect to any database directly.

The checked-in template `config/app.ini` points every service at `http://localhost:8080`. qmake copies it to `config.app.ini` in the build output, where the running app reads it. Treat that file as local runtime configuration: it stores tokens and user details, so keep credentials out of commits.

## What it does

- **Login and session handling**: username/password login via `POST /api/v1/auth/login`, profile lookup via `GET /api/v1/user/{username}`, bearer-token requests, single-flight 401 refresh via `POST /api/v1/auth/refresh-token`, retry of the original request, and token clearing on refresh failure.
- **Catalogue browsing**: tier-scoped categories, products by category, product search, SKU quantity selection, hosted product images with an in-memory image cache, and JSON cache files for categories/products.
- **Multi-order checkout**: several in-memory order tabs, each with its own cart, customer name, service type, discount and note. Tab subtitles show item count and total.
- **Order lifecycle**: confirm `POST /api/v1/order`, pay `PUT /api/v1/order`, void `PUT /api/v1/order/{id}/void`, inspect today's orders, pay `CONFIRMED` orders and void `CONFIRMED`/`PAID` orders from the detail dialog.
- **Payments**: cash, QR and card selection with fixed payment ids `1`, `2` and `3`; cash denominations, amount tendered, change and insufficient-cash validation.
- **Printing**: ESC/POS customer receipt, kitchen slip grouped by category, settlement receipt, and test print through network TCP or the local system printer.
- **Reports**: settlement, daily sales with order volume, and sales-by-category/product screens backed by `/api/v1/reports/**`.
- **Settings**: server host/port, printer type/address/port or system printer names, test print, user/store/brand display and logout.

The menu service also exposes `GET /api/v1/devices/configuration?address=...` for device-specific configuration by globally unique live address. This client does not consume that endpoint yet.

## Tech stack

- C++17 with Qt Widgets; no QML and no Qt Creator-specific runtime dependency.
- qmake project (`desktop-order.pro`); local builds have been generated with Qt 6.8.2 on macOS.
- Qt modules: `core`, `gui`, `network`, `printsupport` and `widgets`.
- `QNetworkAccessManager` for HTTP, `QSettings` for INI state, `QStandardPaths` for cache files, `QTcpSocket`/`QProcess`/Win32 raw printing, and Qt resources for `theme.qss` and the fallback product image.
- macOS and Windows build paths; Windows links `-lwinspool` for raw printing.
- No package-manager lockfile, no third-party libraries and no automated test or lint targets.

## Code layout

The project is a flat root-level source tree; files are grouped here by role.

```text
desktop-order/
├── main.cpp                       # app bootstrap, logging, theme, scaling, busy overlay
├── desktop-order.pro              # qmake project definition
├── config/app.ini                 # endpoint/printer template copied to build output
├── resources.qrc, theme.qss       # embedded assets and global stylesheet
├── apiclient.{h,cpp}              # REST client, bearer auth, 401 refresh/retry
├── tokenmanager.{h,cpp}           # token storage and refresh request
├── setting.{h,cpp}                # INI accessor and file permissions
├── loginscreen.{h,cpp}            # login and profile bootstrap
├── orderscreen.{h,cpp}            # main window, tabs, toolbar, logout
├── orderform.{h,cpp}              # catalogue, search, cart and order composition
├── orderpaymentpopup.{h,cpp}      # confirm/payment dialog and order creation
├── orderpopupwindow.{h,cpp}       # order details, pay and void
├── ordertablewidget.{h,cpp}       # today's order table
├── ordersummary.{h,cpp}           # payment/status totals and top selling products
├── productwidget.{h,cpp}          # product card and image display
├── productdetailpopup.{h,cpp}     # SKU quantity picker
├── imageloader.{h,cpp}            # hosted product-image downloader/cache
├── dailyreportscreen.{h,cpp}      # daily sales and order volume
├── salesreportscreen.{h,cpp}      # sales report
├── settingscreen.{h,cpp}          # server/printer settings and test print
├── orderprint.{h,cpp}             # ESC/POS receipt, kitchen and settlement printing
├── order.*, orderitem.*           # order/cart domain models
├── product.*, sku.*, productimage.*
├── user.*, store.*, chain.*, brand.*
├── cacheutils.{h,cpp}             # JSON cache clearing
├── screenutils.h                  # DPI/font scaling and ORDER_SCALE_FACTOR
├── touchutils.h                   # touch scrolling helpers
└── busyindicator.{h,cpp}          # global API busy overlay
```

`build/` contains generated Makefiles, Qt moc/ui/qrc output, config copies and prebuilt `.app` bundles. Stale files in that directory are not authoritative sources.

## Backend contract

`ApiClient` is a singleton. Every endpoint is a complete URL read from an INI key rather than composed from one base URL. Menu/order/auth responses are expected under `RestAPIResponse.data`; errors use a top-level `message` where available.

| Area | Method and path | Config key |
| --- | --- | --- |
| Auth | `POST /api/v1/auth/login` | `auth/login` |
| Auth | `GET /api/v1/user/{username}` | `auth/user` |
| Auth | `POST /api/v1/auth/refresh-token` | `auth/refresh-token` |
| Menu | `GET /api/v1/category/tier` | `menu/category` |
| Menu | `GET /api/v1/product/category/{id}/price` | `menu/product` |
| Order | `GET /api/v1/order` | `order/daily` |
| Order | `POST /api/v1/order` | `order/confirm` |
| Order | `PUT /api/v1/order` | `order/payment` |
| Order | `PUT /api/v1/order/{id}/void` | `order/void` |
| Reports | `GET /api/v1/reports/settlement?start&end` | `reports/settlement` |
| Reports | `GET /api/v1/reports/daily?start&end` | `reports/daily` |
| Reports | `GET /api/v1/reports/sales?start&end` | `reports/sales` |
| Reports | `GET /api/v1/reports/order-volume?start&end` | `reports/order-volume` |

`Authorization: Bearer <accessToken>` is added to non-auth requests when the host is not empty, `localhost` or `127.0.0.1`. Login and refresh endpoints are excluded. Report requests send `yyyy-MM-ddT00:00:00` and `yyyy-MM-ddT23:59:59` without timezone information.

The order confirm payload sends product/SKU ids, quantities, prices, discount, remark, service type, customer name and `customerId = -1`; totals are computed by the backend. Payment sends only `orderId` and `paymentId`. There is no customer lookup/create flow and no device-registration flow in this client.

## Local state

| Store | Purpose |
| --- | --- |
| `config.app.ini` | absolute endpoint URLs, printer config, tokens and cached user/store/chain/brand details |
| `category_cache.json` | cached category API response under `QStandardPaths::CacheLocation` |
| `product_cache_<categoryId>.json` | cached product API response per category |
| in-memory `Order` objects | open carts and order tabs; no draft persistence |
| `ImageLoader` `QHash` | in-memory hosted product images by URL; not persisted |
| `log.txt`, `log.txt.old` | file logging relative to the current working directory, rotated at 2 MB |
| `printer/type`, `printer/address`, `printer/port`, `printer/receipt`, `printer/kitchen` | network or system printer configuration |
| `ORDER_SCALE_FACTOR` | optional UI scale override used by `ScreenUtils` |

Login, Refresh and logout clear JSON cache files. Logout clears tokens but leaves cached `userDetail.*` values in the INI file.

## Setup

1. Install Qt 6 with Qt Creator, qmake, C++17 compiler support and the Widgets/Network/PrintSupport modules.
2. Open `desktop-order.pro` in Qt Creator and select a desktop kit, or run qmake from a clean build directory.
3. Keep `config/app.ini` as the service/printer template. Do not put real tokens in it; the running app writes tokens into the copied `config.app.ini`.
4. On macOS, the executable expects `config.app.ini` beside the `.app` bundle; qmake's post-link step copies it there. On Windows the equivalent copy is performed with `copy /Y`.
5. If services are split across hosts or ports, edit the copied file directly. The Settings screen derives one host/port from the login URL and rewrites every endpoint to that same base, so it cannot preserve split-service deployments.
6. Network printers need TCP access to `printer/address:printer/port` (default `9100`). System printers use `lp -d <printer> -o raw` on Unix-like systems and Win32 raw printing on Windows.

## Build and run

From a clean build directory:

```bash
qmake ../desktop-order.pro
make
```

With qmake on PATH, the equivalent from the repository root is:

```bash
qmake -o Makefile desktop-order.pro
make
```

The macOS target is `desktop-order.app/Contents/MacOS/desktop-order`. Qt Creator Debug/Release/Profile kits are configured in the local `.pro.user` files; use the `.pro` file as the source of truth, not those machine-specific files.

Generated Makefiles also provide `make clean`, `make distclean`, `make qmake`, `make install`, `make uninstall`, `make dist` and `make distdir`. There is no `test` or lint target.

## Manual verification

There are no automated tests, so verify changes manually:

1. Build and confirm `config.app.ini` was copied into the build output with the intended endpoints.
2. Log in and verify the brand/store header, persisted tokens and profile values.
3. Create multiple tabs; add single-SKU and multi-SKU products; change quantities, customer name, service type, discount and note.
4. Confirm and pay with cash, QR and card; check insufficient cash, change, receipt printing and tab removal.
5. Open a daily order, print receipt/kitchen slips, pay a `CONFIRMED` order and void a `CONFIRMED`/`PAID` order.
6. Open Settlement, Daily Report and Sales Report; check date filters and printed settlement.
7. Change server/printer settings, run Test Print, log out and confirm tokens/cache are cleared.

## Gotchas

- All configured URLs and Settings-generated URLs use plain HTTP; credentials, tokens and order data are unencrypted.
- Tokens and user details are stored in plaintext INI via `QSettings`; `Setting::sync()` restricts the file to owner read/write after writing, but this is not Keychain/DPAPI protection.
- `config/app.ini` and the root `config.app.ini` have drifted: the template uses one port and includes `order/void`/`reports/daily`, while the root copy uses different ports and omits some keys. Only `config/app.ini` is copied by qmake. Missing keys resolve to empty URLs and can produce invalid requests.
- The config path is hard-coded as `applicationDirPath()/../../../config.app.ini`; `Setting::resolveIniPath()` exists but is not used by the constructor.
- `localhost` and `127.0.0.1` requests get no bearer token, including protected endpoints hosted there.
- A 401 refreshes and replays the original request once, including `POST /api/v1/order`; there is no idempotency key, so a lost response can produce a duplicate order.
- Refresh posts the raw refresh-token string with JSON content type rather than a JSON object; refresh reads `accessToken`/`refreshToken` only from the response root, while login accepts them under `data` or at the root.
- HTTP 403 only logs a warning; it does not trigger logout or user-facing re-authentication.
- A successful response that is not a JSON object is treated as an empty-object success.
- Product images are downloaded from `productImage.url` through a separate `ImageLoader` with no auth header; failures fall back silently to the bundled placeholder image.
- Category scrolling is disabled and relies on touch gestures; the Search button is present but filtering is driven by `textChanged`.
- There is no empty-cart or positive-total validation before Confirm; a discount larger than subtotal can produce a negative total.
- Printing uses blocking socket/process/Win32 calls on the GUI thread.
- Cache clearing deletes every `*.json` in both app-data and cache directories, not only this app's cache files.
- Tab labels such as `Order #001` are local session counters, not backend `orderNo` values.
- On payment success, the tab is removed by current index rather than the originating tab pointer.
- The root `.app` artifacts were built against local Qt framework paths and are not self-contained distributables; no signing, notarization or deployment script is configured.
