/*
 * Copyright © 2026-2027 Arktos Browser - IT-Space
 * Copyright 2009 Arora Browser
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

#include <QtTest/QtTest>
#include <searchlineedit.h>

class tst_SearchLineEdit : public QObject
{
    Q_OBJECT

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private slots:
    void searchlineedit_data();
    void searchlineedit();
    void backspaceClearsBaseline();
};

// Subclass that exposes the protected functions.
class SubSearchLineEdit : public SearchLineEdit
{
public:};

// This will be called before the first test function is executed.
// It is only called once.
void tst_SearchLineEdit::initTestCase()
{
}

// This will be called after the last test function is executed.
// It is only called once.
void tst_SearchLineEdit::cleanupTestCase()
{
}

// This will be called before each test function is executed.
void tst_SearchLineEdit::init()
{
}

// This will be called after every test function.
void tst_SearchLineEdit::cleanup()
{
}

void tst_SearchLineEdit::searchlineedit_data()
{
}

void tst_SearchLineEdit::searchlineedit()
{
    SubSearchLineEdit edit;
    QVERIFY(edit.clearButton() != nullptr);
    QVERIFY(edit.searchButton() != nullptr);
}

// SRCH10: Backspace on the field's baseline text (unchanged since it
// was last set) clears the whole line — the one-key clear the url and
// search fields share.  The clear only fires at cursor position 0,
// where a character-wise Backspace would dead-stop; anywhere else the
// key stays character-wise even on pristine baseline text, so a click
// back into a populated field can never wipe it.  Once the user has
// edited, Backspace is character-wise all the way to empty.
void tst_SearchLineEdit::backspaceClearsBaseline()
{
    SubSearchLineEdit edit;

    // Baseline text, cursor homed at 0 (the post-navigation shape):
    // one key clears.
    edit.setText(QLatin1String("baseline"));
    edit.setCursorPosition(0);
    QTest::keyClick(&edit, Qt::Key_Backspace);
    QVERIFY(edit.text().isEmpty());

    // Regression: baseline text with the caret mid-field — the click-
    // back-into-the-url-bar case — must edit character-wise, not wipe.
    edit.setText(QLatin1String("https://example.com/some/path"));
    edit.setCursorPosition(22);
    QTest::keyClick(&edit, Qt::Key_Backspace);
    QCOMPARE(edit.text(), QLatin1String("https://example.com/sme/path"));

    // The same holds at the end of the baseline text.
    edit.setText(QLatin1String("baseline"));
    edit.setCursorPosition(edit.text().size());
    QTest::keyClick(&edit, Qt::Key_Backspace);
    QCOMPARE(edit.text(), QLatin1String("baselin"));

    // After a real user edit backspace stays char-wise, position 0
    // included — the one-key clear is spent.
    edit.clear();
    QTest::keyClicks(&edit, QLatin1String("abc"));
    QTest::keyClick(&edit, Qt::Key_Backspace);
    QCOMPARE(edit.text(), QLatin1String("ab"));

    // Mid-text too — the cursor position doesn't matter once edited.
    edit.setCursorPosition(1);
    QTest::keyClick(&edit, Qt::Key_Backspace);
    QCOMPARE(edit.text(), QLatin1String("b"));

    // Repeated backspace reaches empty.
    edit.setCursorPosition(edit.text().size());
    QTest::keyClick(&edit, Qt::Key_Backspace);
    QVERIFY(edit.text().isEmpty());

    // A programmatic setText re-baselines: homed at 0 the one-key
    // clear applies again.
    edit.setText(QLatin1String("again"));
    edit.setCursorPosition(0);
    QTest::keyClick(&edit, Qt::Key_Backspace);
    QVERIFY(edit.text().isEmpty());
}

QTEST_MAIN(tst_SearchLineEdit)
#include "tst_searchlineedit.moc"

