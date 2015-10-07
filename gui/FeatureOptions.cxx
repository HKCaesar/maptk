/*ckwg +29
 * Copyright 2015 by Kitware, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 *  * Neither name of Kitware, Inc. nor the names of any contributors may be used
 *    to endorse or promote products derived from this software without specific
 *    prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "FeatureOptions.h"

#include "ui_FeatureOptions.h"

#include <vtkActor.h>
#include <vtkProperty.h>

#include <qtUiState.h>
#include <qtUiStateItem.h>

namespace // anonymous
{

//-----------------------------------------------------------------------------
void setColor(vtkActor* actor, QColor const& color)
{
  auto const prop = actor->GetProperty();
  prop->SetColor(color.redF(), color.greenF(), color.blueF());
  prop->SetOpacity(color.alphaF());
}

}

//-----------------------------------------------------------------------------
class FeatureOptionsPrivate
{
public:
  void mapUiState(QString const& key, QSlider* slider);
  void mapUiState(QString const& key, qtColorButton* button);

  Ui::FeatureOptions UI;
  qtUiState uiState;

  QList<vtkActor*> actors;
};

QTE_IMPLEMENT_D_FUNC(FeatureOptions)

//-----------------------------------------------------------------------------
void FeatureOptionsPrivate::mapUiState(
  QString const& key, QSlider* slider)
{
  auto const item = new qtUiState::Item<int, QSlider>(
    slider, &QSlider::value, &QSlider::setValue);
  this->uiState.map(key, item);
}

//-----------------------------------------------------------------------------
void FeatureOptionsPrivate::mapUiState(
  QString const& key, qtColorButton* button)
{
  auto const item = new qtUiState::Item<QColor, qtColorButton>(
    button, &qtColorButton::color, &qtColorButton::setColor);
  this->uiState.map(key, item);
}

//-----------------------------------------------------------------------------
FeatureOptions::FeatureOptions(QString const& settingsGroup,
                               QWidget* parent, Qt::WindowFlags flags)
  : QWidget(parent, flags), d_ptr(new FeatureOptionsPrivate)
{
  QTE_D();

  // Set up UI
  d->UI.setupUi(this);

  // Set up option persistence
  d->uiState.setCurrentGroup(settingsGroup);

  d->mapUiState("Color", d->UI.color);
  d->mapUiState("Size", d->UI.size);

  d->uiState.restore();

  // Connect signals/slots
  connect(d->UI.color, SIGNAL(colorChanged(QColor)),
          this, SLOT(setColor(QColor)));
  connect(d->UI.size, SIGNAL(valueChanged(int)),
          this, SLOT(setSize(int)));
}

//-----------------------------------------------------------------------------
FeatureOptions::~FeatureOptions()
{
  QTE_D();
  d->uiState.save();
}

//-----------------------------------------------------------------------------
void FeatureOptions::setDefaultColor(QColor const& color)
{
  QTE_D();

  d->UI.color->setColor(color);
  d->uiState.restore();
}

//-----------------------------------------------------------------------------
void FeatureOptions::addActor(vtkActor* actor)
{
  QTE_D();

  ::setColor(actor, d->UI.color->color());
  actor->GetProperty()->SetPointSize(d->UI.size->value());

  d->actors.append(actor);
}

//-----------------------------------------------------------------------------
void FeatureOptions::setSize(int size)
{
  QTE_D();

  foreach (auto const actor, d->actors)
  {
    actor->GetProperty()->SetPointSize(size);
  }

  emit this->modified();
}

//-----------------------------------------------------------------------------
void FeatureOptions::setColor(QColor const& color)
{
  QTE_D();

  foreach (auto const actor, d->actors)
  {
    ::setColor(actor, color);
  }

  emit this->modified();
}
