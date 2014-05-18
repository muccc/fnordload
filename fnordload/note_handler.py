from . import NoteValidator
from . import account

class NoteHandler(NoteValidator):
    def __init__(self, *args, **kwargs):
        NoteValidator.__init__(self, *args, **kwargs)
        self._accounts = {}
    #def __init__(inhibits = [1, 1, 1, 0, 0, 0])
    #    self._note_validator = fnordload.NoteValidator(
    #            device = eSSPport, inhibits = inhibits)
    #    NoteValidator.__init__(inhibits = inhibits)

    def _check_account(self, account_name):
         if account_name not in self._accounts:
            self._accounts[account_name] = account.Account(account_name)
       
    def read_note(self, account_name):
        amount = NoteValidator.read_note(self)

        #self._add_to_account(account, amount)
        self._check_account(account_name)
        self._accounts[account_name].add(amount)

        return amount

    def account_value(self, account_name):
        self._check_account(account_name)
        return self._accounts[account_name].value

    def payout(self, account_name, value):
        self._check_account(account_name)
        self._accounts[account_name].subtract(value)
