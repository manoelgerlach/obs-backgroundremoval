// SPDX-FileCopyrightText: 2026 Kaito Udagawa <umireon@kaito.tokyo>
// SPDX-FileCopyrightText: 2026 Manoel Gerlach <mail@manoel.us>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QDialog>

class QDialogButtonBox;
class QCheckBox;
class QFrame;
class QLabel;
class QPlainTextEdit;
class QPushButton;
class QString;

class AboutDialog : public QDialog {
public:
	explicit AboutDialog(const QString &version, QWidget *parent = nullptr, bool updateNotificationsEnabled = false,
			     bool updateFeedAvailable = false);
	~AboutDialog() noexcept override;
	bool updateNotificationsEnabled() const;

private:
	void setupUi(const QString &version, bool updateNotificationsEnabled, bool updateFeedAvailable);

	QLabel *const logoLabel;
	QLabel *const productNameLabel;
	QLabel *const authorLabel;
	QLabel *const versionLabel;
	QLabel *const officialSiteLink;
	QLabel *const githubLink;
	QLabel *const communityLink;
	QLabel *const obsForumLink;
	QFrame *const headerSeparator;
	QLabel *const descriptionLabel;
	QLabel *const copyrightLabel;
	QLabel *const legalNoticeLabel;
	QFrame *const updateNotificationFrame;
	QCheckBox *const enableUpdateNotifications;
	QLabel *const updateCheckDescription;
	QLabel *const updateCheckUrlLabel;
	QDialogButtonBox *const buttonBox;
	QPushButton *const licensesButton;
	QPushButton *const aboutQtButton;
	QDialog *const licensesDialog;
	QPlainTextEdit *const licensesTextEdit;
	QDialogButtonBox *const licensesButtonBox;
};
