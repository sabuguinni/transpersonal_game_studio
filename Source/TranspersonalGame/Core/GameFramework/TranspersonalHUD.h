// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include \"CoreMinimal.h\"
#include \"GameFramework/HUD.h\"
#include \"TranspersonalHUD.generated.h\"

DECLARE_LOG_CATEGORY_EXTERN(LogTranspersonalHUD, Log, All);

class ATranspersonalCharacter;
class UUserWidget;

/**
 * Transpersonal HUD - Survival-Focused Interface
 * 
 * Designed to maintain immersion while providing essential survival information:
 * - Minimal UI elements to avoid clutter
 * - Survival stats (health, stamina, hunger, thirst)
 * - Fear indicator system
 * - Threat warnings
 * - Optional debug information
 * 
 * Design Philosophy:
 * - Less is more - only show what's essential
 * - Fear should be communicated through visual effects
 * - UI should never break immersion
 */
UCLASS()
class TRANSPERSONALGAME_API ATranspersonalHUD : public AHUD
{
    GENERATED_BODY()

public:
    ATranspersonalHUD();

protected:
    virtual void BeginPlay() override;
    virtual void DrawHUD() override;

public:
    // HUD Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"HUD Settings\")
    bool bShowDebugInfo;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"HUD Settings\")
    bool bShowSurvivalStats;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"HUD Settings\")
    bool bShowFearIndicator;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"HUD Settings\")
    bool bMinimalHUD;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"HUD Settings\")
    float HUDOpacity;
    
    // Widget Classes
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Widgets\")
    TSubclassOf<UUserWidget> SurvivalStatsWidgetClass;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Widgets\")
    TSubclassOf<UUserWidget> CraftingMenuWidgetClass;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Widgets\")
    TSubclassOf<UUserWidget> InventoryWidgetClass;
    
    // Color Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Colors\")
    FLinearColor HealthColor;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Colors\")
    FLinearColor StaminaColor;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Colors\")
    FLinearColor HungerColor;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Colors\")
    FLinearColor ThirstColor;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Colors\")
    FLinearColor FearColor;

    // HUD Control Functions
    UFUNCTION(BlueprintCallable, Category = \"HUD\")
    void ToggleDebugInfo();
    
    UFUNCTION(BlueprintCallable, Category = \"HUD\")
    void ToggleMinimalHUD();
    
    UFUNCTION(BlueprintCallable, Category = \"HUD\")
    void SetHUDOpacity(float NewOpacity);

protected:
    // Widget References
    UPROPERTY()
    UUserWidget* SurvivalStatsWidget;
    
    UPROPERTY()
    UUserWidget* CraftingMenuWidget;
    
    UPROPERTY()
    UUserWidget* InventoryWidget;

    // Drawing Functions
    void CreateSurvivalStatsWidget();
    void DrawSurvivalStats(ATranspersonalCharacter* PlayerCharacter);
    void DrawFearIndicator(ATranspersonalCharacter* PlayerCharacter);
    void DrawThreatWarning();
    void DrawDebugInfo(ATranspersonalCharacter* PlayerCharacter);
    
    // Helper Functions
    void DrawStatBar(float X, float Y, float Width, float Height, 
                    float Percentage, const FLinearColor& Color, const FString& Label);
};"