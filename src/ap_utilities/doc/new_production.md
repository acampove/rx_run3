# Creating a new production

The _template_ branch needed is [here](https://gitlab.cern.ch/lhcb-datapkg/AnalysisProductions/-/tree/rquaglia_rd_ap_2024_2025)

- Clone it with:

```bash
git clone ssh://git@gitlab.cern.ch:7999/lhcb-datapkg/AnalysisProductions.git
```

- Prepare new production 

```bash
mv rd_ap_2024_2025 new_production
cd new_production

# This will change every occurrence of rd_ap_2024_2025 with new_production in the code
grep -rl 'new_production' rd_ap_2024_2025/ | xargs sed -i 's/new_production/rd_ap_2024/g'
```

- Modify the code
- Push and open an MR

```bash
git add .
git commit -m "Some messge"
git push origin new_production
```
