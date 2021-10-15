from generate_config_instructions import convert_datatime_to_special_epoch_min,\
    gen_instruction,convert_special_epoch_min_to_datetime
import unittest
from datetime import datetime

class Test_Scorer(unittest.TestCase):

    def test_gen_instruction(self):
        res = gen_instruction(519506,519524)
        self.assertEqual("64ed070052ed0700",res)

    def test_gen_instruction_error_wrong_size(self):
        self.assertRaises(ValueError, gen_instruction, 5,4)

    def test_gen_instruction_error_equal(self):
        self.assertRaises(ValueError, gen_instruction, 4,4)

    def test_convert_datatime_to_special_epoch_min(self):
        date_time_str = '2020-11-26 19:30:57.084'
        date_time_older = datetime.strptime(date_time_str, '%Y-%m-%d %H:%M:%S.%f')
        res = convert_datatime_to_special_epoch_min(date_time_older)
        self.assertEqual(476309,res)

    def test_convert_special_epoch_min_to_datetime(self):
        res = convert_special_epoch_min_to_datetime(476309)
        self.assertEqual("2020-11-26 19:30:01",res)


if __name__ == '__main__':
    unittest.main()