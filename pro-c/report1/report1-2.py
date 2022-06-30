import sys
from jinja2 import Environment

template = r'''
\documentclass[uplatex, dvipdfmx, 10pt, a4paper]{jsarticle}
\usepackage{array}
\usepackage[labelformat=empty,labelsep=none]{caption}
\usepackage{multirow}
\newcommand{\mcl}[1]{\multirow{2}{*}{#1}}
\begin{document}
{% for d in data %}
\begin{table*}[htbp]
    \centering
    \caption{\underline{[[d.title]] - [[d.sub_title]] -}}
    \begin{tabular*}{\linewidth}{wc{2cm}|wc{2cm}|wc{2cm}|wc{2cm}|wc{2cm}|wc{0.8cm}|wc{0.8cm}|wc{0.8cm}} \hline
        & {% for country in d.countries %} [[country]] & {% endfor %} win & draw & lose\\ \hline
        {% for coutry, score in d.scores.items() %}
        \mcl{[[coutry]]} {% for r in score[0::2] %} & {% if r == 'W' or r == 'L' or r == 'D' %} [[r]] {% else %} \mcl{[[r]]} {% endif %} {% endfor %} \\
                         {% for r in score[1::2] %} &  [[r]] {% endfor %} \\ \hline
        {% endfor %}
    \end{tabular*}
\end{table*}

\newpage
{% endfor %}
\end{document}
'''


class Data:
    title: str
    sub_title: str
    countries: list
    scores: dict
    results: dict

    def __init__(self, lines: list):
        self.title = lines[0]
        self.sub_title = lines[2]
        buf = []
        for coutry in lines[3].split():
            i = coutry.find('_')
            if (i != -1):
                buf.append(coutry[:i] + '\\' + coutry[i:])
            else:
                buf.append(coutry)
        self.countries = buf
        self.scores = {}
        self.results = {}
        for i, coutry in enumerate(self.countries):
            buf = []
            win = 0
            lose = 0
            draw = 0
            for s in lines[i + 4].split()[1:]:
                if (s == 'O'):
                    buf.append('W')
                    win += 1
                elif (s == 'X'):
                    buf.append('L')
                    lose += 1
                elif (s == '='):
                    buf.append('D')
                    draw += 1
                elif (s == '#'):
                    buf.extend([' ', ' '])
                else:
                    buf.append(s)
            buf.extend([str(win), ' ', str(draw), ' ', str(lose), ' '])
            self.scores[coutry] = buf

    def format_dic(self):
        dic = {}
        dic["title"] = self.title
        dic["sub_title"] = self.sub_title
        dic["countries"] = self.countries
        dic["scores"] = self.scores
        return dic


def is_requirements(str: str) -> bool:
    if (str >= 'A' and str <= 'H' and len(str) == 1):
        return True
    return False


def error_exit(str: str) -> None:
    print(str, file=sys.stderr)
    sys.exit(1)


def parse_commandline_argument(lt: list) -> list:
    lt = list()
    # Commandline arguments : more than 8 arguments
    if len(sys.argv[1:]) > 8:
        error_exit(
            'Commandline arguments : too many arguments. arguments is 8 or less.')
    for argv in sys.argv[1:]:
        # Commandline arguments : not meet requirements
        if (not is_requirements(argv)):
            error_exit('Commandline arguments : not meet requirements')
            sys.exit(1)
        for l in lt:
            # Commandline arguments : already have the same arguments
            if argv == l:
                error_exit(
                    'Commandline arguments : ' + argv + ' already have the same arguments')
        lt.append(argv)
    return lt


def parse_group_file(group: str):
    filename = './report1-2_data/group' + group
    file = open(filename)
    data = file.read()
    file.close()
    return data.splitlines()


def main():
    group_list = ['A', 'B', 'C', 'D', 'E', 'F', 'G', 'H']
    if (len(sys.argv) != 1):
        group_list = parse_commandline_argument(group_list)

    data = []
    for group in group_list:
        lines = parse_group_file(group)
        data.append(Data(lines).format_dic())

    env = Environment(
        variable_start_string="[[",
        variable_end_string="]]",
        comment_start_string='\#{',
        comment_end_string='}'
    )
    temp = env.from_string(template)

    file = open('group-' + "".join(group_list) + ".tex", 'wt')
    file.write(temp.render({"data": data}))
    file.close()


if __name__ == '__main__':
    main()
