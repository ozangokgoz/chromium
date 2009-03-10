// Copyright (c) 2009 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/gtk/download_shelf_gtk.h"

#include "base/logging.h"
#include "chrome/browser/download/download_item_model.h"
#include "chrome/browser/gtk/custom_button.h"
#include "chrome/browser/gtk/download_item_gtk.h"
#include "chrome/common/l10n_util.h"
#include "chrome/common/resource_bundle.h"
#include "grit/generated_resources.h"
#include "grit/theme_resources.h"

// TODO(port): remove this after tab_contents.h is ported.
#include "chrome/common/temp_scaffolding_stubs.h"

namespace {

// The height of the download items. Should be 28, as that is the height of
// their bitmaps.
const int kDownloadItemHeight = 28;

// Padding between the download widgets.
const int kDownloadItemPadding = 10;

// Padding between the top/bottom of the download widgets and the edge of the
// shelf.
const int kTopBottomPadding = 4;

// Padding between the left side of the shelf and the first download item.
const int kLeftPadding = 2;

// Padding between the right side of the shelf and the close button.
const int kRightPadding = 10;

// The background color of the shelf.
static GdkColor kBackgroundColor = { 0, 230 * 257, 237 * 257, 244 * 257 };

// Border color (the top pixel of the shelf).
static GdkColor kBorderColor = { 0, 214 * 257, 214 * 257, 214 * 257 };

}

// static
DownloadShelf* DownloadShelf::Create(TabContents* tab_contents) {
  return new DownloadShelfGtk(tab_contents);
}

DownloadShelfGtk::DownloadShelfGtk(TabContents* tab_contents)
    : DownloadShelf(tab_contents),
      is_showing_(false) {
  // Logically, the shelf is a vbox that contains two children: a one pixel
  // tall event box, which serves as the top border, and an hbox, which holds
  // the download items and other shelf widgets (close button, show-all-
  // downloads link).
  // To make things pretty, we have to add a few more widgets. To get padding
  // right, we stick the hbox in an alignment. We put that alignment in an
  // event box so we can color the background.

  // Create the top border.
  GtkWidget* top_border = gtk_event_box_new();
  gtk_widget_set_size_request(GTK_WIDGET(top_border), 0, 1);
  gtk_widget_modify_bg(top_border, GTK_STATE_NORMAL, &kBorderColor);

  // Create |hbox_|.
  hbox_ = gtk_hbox_new(FALSE, kDownloadItemPadding);
  gtk_widget_set_size_request(hbox_, -1, kDownloadItemHeight);

  // Get the padding and background color for |hbox_| right.
  GtkWidget* padding = gtk_alignment_new(0, 0, 1, 1);
  // Subtract 1 from top spacing to account for top border.
  gtk_alignment_set_padding(GTK_ALIGNMENT(padding),
      kTopBottomPadding - 1, kTopBottomPadding, kLeftPadding, kRightPadding);
  GtkWidget* padding_bg = gtk_event_box_new();
  gtk_container_add(GTK_CONTAINER(padding_bg), padding);
  gtk_container_add(GTK_CONTAINER(padding), hbox_);
  gtk_widget_modify_bg(padding_bg, GTK_STATE_NORMAL, &kBackgroundColor);

  shelf_ = gtk_vbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(shelf_), top_border, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(shelf_), padding_bg, FALSE, FALSE, 0);

  // Create and pack the close button.
  close_button_.reset(new CustomDrawButton(IDR_CLOSE_BAR,
                      IDR_CLOSE_BAR_P, IDR_CLOSE_BAR_H, 0));
  g_signal_connect(G_OBJECT(close_button_->widget()), "clicked",
                   G_CALLBACK(OnCloseButtonClick), this);
  GTK_WIDGET_UNSET_FLAGS(close_button_->widget(), GTK_CAN_FOCUS);
  GtkWidget* centering_vbox = gtk_vbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(centering_vbox),
                     close_button_->widget(), TRUE, FALSE, 0);
  gtk_box_pack_end(GTK_BOX(hbox_), centering_vbox, FALSE, FALSE, 0);

  // Stick ourselves at the bottom of the parent tab contents.
  GtkWidget* parent_contents = tab_contents->GetNativeView();
  gtk_box_pack_end(GTK_BOX(parent_contents), shelf_, FALSE, FALSE, 0);
  Show();
}

DownloadShelfGtk::~DownloadShelfGtk() {
}

void DownloadShelfGtk::AddDownload(BaseDownloadItemModel* download_model_) {
  // TODO(estade): we need to delete these at some point. There's no explicit
  // mass delete on windows, figure out where they do it.
  download_items_.push_back(new DownloadItemGtk(download_model_, hbox_));
  Show();
}

bool DownloadShelfGtk::IsShowing() const {
  return is_showing_;
}

void DownloadShelfGtk::Show() {
  if (is_showing_)
    return;

  gtk_widget_show_all(shelf_);
  is_showing_ = true;
}

void DownloadShelfGtk::Hide() {
  if (!is_showing_)
    return;

  gtk_widget_hide_all(shelf_);
  is_showing_ = false;
}

// static
void DownloadShelfGtk::OnCloseButtonClick(GtkWidget* button,
                                          DownloadShelfGtk* shelf) {
  shelf->Hide();
}
