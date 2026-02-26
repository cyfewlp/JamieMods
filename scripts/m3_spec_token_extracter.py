# Internal helper script - Personal Use Only.
# -----------------------------------------------------------------------------
# This is a quick-and-dirty utility maintained for specific local tasks.
# It is NOT a production-grade tool and will not receive regular updates.
# If it breaks, you get to keep both pieces.
# -----------------------------------------------------------------------------

import asyncio
import re

from playwright.async_api import async_playwright


async def scrape_m3_tokens(url, component_name):
    async with async_playwright() as p:
        browser = await p.chromium.launch(headless=True)
        context = await browser.new_context(viewport={'width': 1920, 'height': 1080})
        page = await context.new_page()

        print(f"Accessing: {url}...")
        await page.goto(url, wait_until="networkidle")

        index = 0
        for token_viewer in await page.locator("token-viewer").all():
            design_system_data = await token_viewer.get_attribute("design-system-data")
            file_name = f"build/m3_specs/m3_{component_name}_spec_data_{index}.json"
            with open(file_name, "w", encoding="utf-8") as f:
                f.write(design_system_data)
                print("Already written spec data to " + file_name)
            index += 1

        await browser.close()


async def main(url : str, component_name):
    await scrape_m3_tokens(url, component_name)

async def main_with_url_parsing(url : str):
    component_name = url[url.rfind("/components/") + len("/components/"):url.rfind("/specs")]
    await main(url, component_name)


async def scan_components(all_components_url):
    async with async_playwright() as p:
        browser = await p.chromium.launch(headless=True)
        context = await browser.new_context(viewport={'width': 1920, 'height': 1080})
        page = await context.new_page()

        print(f"Accessing: {all_components_url}...")
        await page.goto(all_components_url, wait_until="networkidle")

        nav = page.locator("#navigationDrawer")

        component_spec_url_list = []
        for link in await nav.get_by_role("treeitem").all():
            href = await link.get_attribute("href")
            match = re.search(r"/components/(\w+)", href)
            if match:
                component_spec_url_list.append({"url": f"https://m3.material.io{href}/specs", "name": match.group(1)})
        for component_spec_url in component_spec_url_list:
            await main(component_spec_url["url"], component_spec_url["name"])


if __name__ == "__main__":
    asyncio.run(scan_components("https://m3.material.io/components"))
