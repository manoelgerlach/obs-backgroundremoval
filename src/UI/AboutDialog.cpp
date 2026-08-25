// SPDX-FileCopyrightText: 2026 Kaito Udagawa <umireon@kaito.tokyo>
// SPDX-FileCopyrightText: 2026 Manoel Gerlach <mail@manoel.us>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AboutDialog.hpp"

#if __has_include(<legal.hpp>)
#include <legal.hpp>
#define ABOUT_DIALOG_HAS_LEGAL_TEXT
#endif

#include <QDialogButtonBox>
#include <QCheckBox>
#include <QFont>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPixmap>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>

AboutDialog::AboutDialog(const QString &version, QWidget *parent, bool updateNotificationsEnabled,
			 bool updateFeedAvailable)
	: QDialog(parent),
	  logoLabel(new QLabel(this)),
	  productNameLabel(new QLabel(this)),
	  authorLabel(new QLabel(this)),
	  versionLabel(new QLabel(this)),
	  officialSiteLink(new QLabel(this)),
	  githubLink(new QLabel(this)),
	  communityLink(new QLabel(this)),
	  obsForumLink(new QLabel(this)),
	  headerSeparator(new QFrame(this)),
	  descriptionLabel(new QLabel(this)),
	  copyrightLabel(new QLabel(this)),
	  legalNoticeLabel(new QLabel(this)),
	  updateNotificationFrame(new QFrame(this)),
	  enableUpdateNotifications(new QCheckBox(updateNotificationFrame)),
	  updateCheckDescription(new QLabel(updateNotificationFrame)),
	  updateCheckUrlLabel(new QLabel(updateNotificationFrame)),
	  buttonBox(new QDialogButtonBox(QDialogButtonBox::Ok, this)),
	  licensesButton(new QPushButton(this)),
	  aboutQtButton(new QPushButton(this)),
	  licensesDialog(new QDialog(this)),
	  licensesTextEdit(new QPlainTextEdit(licensesDialog)),
	  licensesButtonBox(new QDialogButtonBox(QDialogButtonBox::Close, licensesDialog))
{
	setupUi(version, updateNotificationsEnabled, updateFeedAvailable);
}

AboutDialog::~AboutDialog() noexcept = default;

bool AboutDialog::updateNotificationsEnabled() const
{
	return enableUpdateNotifications->isChecked();
}

void AboutDialog::setupUi(const QString &version, bool updateNotificationsEnabled, bool updateFeedAvailable)
{
	setWindowTitle(tr("About obs-backgroundremoval"));
	setModal(true);

	auto *rootLayout = new QVBoxLayout(this);
	rootLayout->setSizeConstraint(QLayout::SetFixedSize);
	rootLayout->addStrut(640);
	auto *headerLayout = new QHBoxLayout;

	QPixmap logo(QStringLiteral(":/obs-backgroundremoval-resources/logo.png"));
	if (!logo.isNull()) {
		logoLabel->setPixmap(logo.scaled(96, 96, Qt::KeepAspectRatio, Qt::SmoothTransformation));
	}
	logoLabel->setAlignment(Qt::AlignCenter);
	headerLayout->addWidget(logoLabel);

	auto *productLayout = new QVBoxLayout;
	productNameLabel->setText(tr("obs-backgroundremoval — Windows DirectML fork"));
	QFont productNameFont = productNameLabel->font();
	productNameFont.setPointSizeF(productNameFont.pointSizeF() * 1.5);
	productNameFont.setBold(true);
	productNameLabel->setFont(productNameFont);
	productNameLabel->setWordWrap(true);
	productLayout->addWidget(productNameLabel);

	const QString royshilLink =
		QStringLiteral("<a href=\"https://github.com/royshil\">%1</a>").arg(tr("Roy Shilkrot (royshil)"));
	const QString umireonLink =
		QStringLiteral("<a href=\"https://github.com/umireon\">%1</a>").arg(tr("Kaito Udagawa (umireon)"));
	authorLabel->setText(tr("Original authors: %1 and %2").arg(royshilLink, umireonLink));
	productLayout->addWidget(authorLabel);
	versionLabel->setText(tr("Version %1").arg(version));
	productLayout->addWidget(versionLabel);
	headerLayout->addLayout(productLayout, 1);
	rootLayout->addLayout(headerLayout);

	headerSeparator->setFrameShape(QFrame::HLine);
	headerSeparator->setFrameShadow(QFrame::Sunken);
	rootLayout->addWidget(headerSeparator);

	const QString manoelLink =
		QStringLiteral("<a href=\"https://github.com/manoelgerlach\">%1</a>").arg(tr("Manoel Gerlach"));
	descriptionLabel->setText(
		tr("<b>Unofficial modified fork</b><br><br>"
		   "This build is based on the open-source obs-backgroundremoval project by Roy Shilkrot and "
		   "Kaito Udagawa and is not an official upstream release.<br><br>"
		   "This fork restores ONNX Runtime DirectML GPU inference support on Windows and includes "
		   "additional Windows-specific build, packaging, diagnostics, and validation changes.<br><br>"
		   "Fork maintainer and modifications (2026): %1")
			.arg(manoelLink));
	descriptionLabel->setTextFormat(Qt::RichText);
	descriptionLabel->setWordWrap(true);
	rootLayout->addWidget(descriptionLabel);

	auto *linksLayout = new QVBoxLayout;
	officialSiteLink->setText(
		QStringLiteral("<a href=\"https://github.com/manoelgerlach/obs-backgroundremoval\">%1</a>")
			.arg(tr("Fork Repository")));
	linksLayout->addWidget(officialSiteLink);
	githubLink->setText(QStringLiteral("<a href=\"https://github.com/royshil/obs-backgroundremoval\">%1</a>")
				    .arg(tr("Original / Upstream Project")));
	linksLayout->addWidget(githubLink);
	communityLink->setText(QStringLiteral("<a href=\"https://royshil.github.io/obs-backgroundremoval/\">%1</a>")
				       .arg(tr("Original Project Site")));
	linksLayout->addWidget(communityLink);
	obsForumLink->setText(QStringLiteral("<a href=\"https://obsproject.com/forum/resources/"
					     "background-removal-virtual-green-screen-low-light-enhance.1260/\">%1</a>")
				      .arg(tr("OBS Forum")));
	linksLayout->addWidget(obsForumLink);
	rootLayout->addLayout(linksLayout);

	updateNotificationFrame->setAutoFillBackground(true);
	updateNotificationFrame->setBackgroundRole(QPalette::Base);
	auto *updateNotificationLayout = new QVBoxLayout(updateNotificationFrame);

	enableUpdateNotifications->setText(
		updateFeedAvailable ? tr("Notify me about new fork versions on the Filter Properties screen")
				    : tr("Automatic update checking is disabled for this fork"));
	QFont updateNotificationHeadingFont = enableUpdateNotifications->font();
	updateNotificationHeadingFont.setBold(true);
	enableUpdateNotifications->setFont(updateNotificationHeadingFont);
	enableUpdateNotifications->setChecked(updateFeedAvailable && updateNotificationsEnabled);
	enableUpdateNotifications->setEnabled(updateFeedAvailable);
	updateNotificationLayout->addWidget(enableUpdateNotifications);

	const QString updateCheckDescriptionText =
		updateFeedAvailable ? tr("When OBS Studio starts, the plugin checks the configured fork "
					 "release feed. The plugin never downloads or installs updates.")
				    : tr("No fork release feed is configured. This build does not query "
					 "the original project's release metadata or perform automatic update "
					 "checks. Use the repository links above to review fork and upstream "
					 "project information.");
	updateCheckDescription->setText(QStringLiteral("<div style=\"line-height: 80%;\">%1</div>")
						.arg(updateCheckDescriptionText.toHtmlEscaped()));
	updateCheckDescription->setTextFormat(Qt::RichText);
	updateCheckDescription->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
	updateCheckDescription->setWordWrap(true);
	updateCheckDescription->setAlignment(Qt::AlignLeft | Qt::AlignTop);
	updateCheckDescription->setContentsMargins(24, 0, 0, 0);
	updateNotificationLayout->addWidget(updateCheckDescription);

	updateCheckUrlLabel->setVisible(false);
	rootLayout->addWidget(updateNotificationFrame);

	copyrightLabel->setText(tr("Copyright © 2021–2026 Roy Shilkrot<br>"
				   "Copyright © 2023–2026 Kaito Udagawa<br>"
				   "Windows DirectML fork modifications © 2026 Manoel Gerlach"));
	copyrightLabel->setTextFormat(Qt::RichText);
	copyrightLabel->setWordWrap(true);
	rootLayout->addWidget(copyrightLabel);

	legalNoticeLabel->setText(
		tr("Licensed under the GNU General Public License v3.0 or later. This plugin includes "
		   "third-party software and pretrained models provided under their respective licenses."));
	legalNoticeLabel->setWordWrap(true);
	rootLayout->addWidget(legalNoticeLabel);

	for (QLabel *label : {productNameLabel, copyrightLabel, legalNoticeLabel}) {
		label->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
	}

	for (QLabel *link :
	     {authorLabel, descriptionLabel, versionLabel, officialSiteLink, githubLink, communityLink, obsForumLink}) {
		link->setTextFormat(Qt::RichText);
		link->setTextInteractionFlags(Qt::TextBrowserInteraction | Qt::TextSelectableByKeyboard);
		link->setOpenExternalLinks(true);
	}

	rootLayout->addStretch();

#ifdef ABOUT_DIALOG_HAS_LEGAL_TEXT
	licensesDialog->setWindowTitle(tr("Licenses"));
	licensesDialog->setModal(false);
	auto *licensesLayout = new QVBoxLayout(licensesDialog);
	licensesTextEdit->setReadOnly(true);
	licensesTextEdit->setPlainText(
		QString::fromUtf8(reinterpret_cast<const char *>(obs_backgroundremoval::legal_text),
				  static_cast<qsizetype>(obs_backgroundremoval::legal_text_size)));
	licensesLayout->addWidget(licensesTextEdit);
	licensesLayout->addWidget(licensesButtonBox);
	connect(licensesButtonBox, &QDialogButtonBox::rejected, licensesDialog, &QDialog::reject);

	licensesButton->setText(tr("Licenses"));
	buttonBox->addButton(licensesButton, QDialogButtonBox::ActionRole);
	connect(licensesButton, &QPushButton::clicked, licensesDialog, [dialog = licensesDialog] {
		dialog->showMaximized();
		dialog->raise();
		dialog->activateWindow();
	});
#endif
	aboutQtButton->setText(tr("About Qt"));
	buttonBox->addButton(aboutQtButton, QDialogButtonBox::ActionRole);
	connect(aboutQtButton, &QPushButton::clicked, this, [this] { QMessageBox::aboutQt(this); });
	connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
	rootLayout->addWidget(buttonBox);
}
