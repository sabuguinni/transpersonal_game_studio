// Copyright Transpersonal Game Studio. All Rights Reserved.

#include \"TranspersonalHUD.h\"
#include \"Engine/Engine.h\"
#include \"Engine/Canvas.h\"
#include \"Engine/Texture2D.h\"
#include \"UObject/ConstructorHelpers.h\"
#include \"Blueprint/UserWidget.h\"
#include \"TranspersonalCharacter.h\"
#include \"../StudioDirectorSubsystem.h\"

DEFINE_LOG_CATEGORY(LogTranspersonalHUD);

ATranspersonalHUD::ATranspersonalHUD()
{
    // Initialize HUD settings
    bShowDebugInfo = false;
    bShowSurvivalStats = true;
    bShowFearIndicator = true;
    bMinimalHUD = true; // Minimal HUD for immersion
    
    // Set default colors
    HealthColor = FLinearColor::Red;
    StaminaColor = FLinearColor::Yellow;
    HungerColor = FLinearColor(1.0f, 0.5f, 0.0f); // Orange
    ThirstColor = FLinearColor::Blue;
    FearColor = FLinearColor(0.5f, 0.0f, 0.5f); // Purple
    
    // Initialize opacity
    HUDOpacity = 0.8f;
}

void ATranspersonalHUD::BeginPlay()
{
    Super::BeginPlay();
    
    // Create survival stats widget
    CreateSurvivalStatsWidget();
    
    // Register with Studio Director
    if (UStudioDirectorSubsystem* StudioDirector = GetWorld()->GetSubsystem<UStudioDirectorSubsystem>())
    {
        FSystemInfo HUDSystemInfo;
        HUDSystemInfo.SystemName = TEXT(\"PlayerHUD\");
        HUDSystemInfo.AgentName = TEXT(\"Studio Director\");
        HUDSystemInfo.AgentNumber = 1;
        HUDSystemInfo.Status = ESystemStatus::Online;
        HUDSystemInfo.Priority = EStudioDirectorPriority::Medium;
        
        StudioDirector->RegisterSystem(HUDSystemInfo);
    }
    
    UE_LOG(LogTranspersonalHUD, Warning, TEXT(\"Transpersonal HUD initialized\"));
}

void ATranspersonalHUD::DrawHUD()
{
    Super::DrawHUD();
    
    if (!bMinimalHUD)
    {
        return; // Skip HUD drawing if minimal mode is enabled
    }
    
    // Get player character
    ATranspersonalCharacter* PlayerCharacter = Cast<ATranspersonalCharacter>(GetOwningPawn());
    if (!PlayerCharacter)
    {
        return;
    }
    
    // Draw survival stats
    if (bShowSurvivalStats)
    {
        DrawSurvivalStats(PlayerCharacter);
    }
    
    // Draw fear indicator
    if (bShowFearIndicator)
    {
        DrawFearIndicator(PlayerCharacter);
    }
    
    // Draw debug info if enabled
    if (bShowDebugInfo)
    {
        DrawDebugInfo(PlayerCharacter);
    }
    
    // Draw threat warning
    if (PlayerCharacter->bNearbyThreat)
    {
        DrawThreatWarning();
    }
}

void ATranspersonalHUD::CreateSurvivalStatsWidget()
{
    if (SurvivalStatsWidgetClass)
    {
        SurvivalStatsWidget = CreateWidget<UUserWidget>(GetWorld(), SurvivalStatsWidgetClass);
        if (SurvivalStatsWidget)
        {
            SurvivalStatsWidget->AddToViewport();
            UE_LOG(LogTranspersonalHUD, Log, TEXT(\"Survival stats widget created\"));
        }
    }
}

void ATranspersonalHUD::DrawSurvivalStats(ATranspersonalCharacter* PlayerCharacter)
{
    if (!PlayerCharacter || !Canvas)
    {
        return;
    }
    
    // Position for stats (top-left corner)
    float StartX = 50.0f;
    float StartY = 50.0f;
    float BarWidth = 200.0f;
    float BarHeight = 20.0f;
    float BarSpacing = 30.0f;
    
    // Draw health bar
    DrawStatBar(StartX, StartY, BarWidth, BarHeight, 
                PlayerCharacter->GetHealthPercentage(), 
                HealthColor, TEXT(\"Health\"));
    
    // Draw stamina bar
    DrawStatBar(StartX, StartY + BarSpacing, BarWidth, BarHeight, 
                PlayerCharacter->GetStaminaPercentage(), 
                StaminaColor, TEXT(\"Stamina\"));
    
    // Draw hunger bar
    DrawStatBar(StartX, StartY + (BarSpacing * 2), BarWidth, BarHeight, 
                PlayerCharacter->GetHungerPercentage(), 
                HungerColor, TEXT(\"Hunger\"));
    
    // Draw thirst bar
    DrawStatBar(StartX, StartY + (BarSpacing * 3), BarWidth, BarHeight, 
                PlayerCharacter->GetThirstPercentage(), 
                ThirstColor, TEXT(\"Thirst\"));
}

void ATranspersonalHUD::DrawStatBar(float X, float Y, float Width, float Height, 
                                   float Percentage, const FLinearColor& Color, const FString& Label)
{
    if (!Canvas)
    {
        return;
    }
    
    // Draw background
    FCanvasTileItem BackgroundItem(FVector2D(X, Y), FVector2D(Width, Height), FLinearColor(0.0f, 0.0f, 0.0f, 0.5f * HUDOpacity));
    BackgroundItem.BlendMode = SE_BLEND_Translucent;
    Canvas->DrawItem(BackgroundItem);
    
    // Draw fill
    float FillWidth = Width * FMath::Clamp(Percentage, 0.0f, 1.0f);
    FCanvasTileItem FillItem(FVector2D(X, Y), FVector2D(FillWidth, Height), Color * HUDOpacity);
    FillItem.BlendMode = SE_BLEND_Translucent;
    Canvas->DrawItem(FillItem);
    
    // Draw border
    FCanvasBoxItem BorderItem(FVector2D(X, Y), FVector2D(Width, Height));
    BorderItem.SetColor(FLinearColor::White * HUDOpacity);
    Canvas->DrawItem(BorderItem);
    
    // Draw label
    FCanvasTextItem TextItem(FVector2D(X, Y - 20.0f), FText::FromString(Label), GEngine->GetSmallFont(), FLinearColor::White * HUDOpacity);
    Canvas->DrawItem(TextItem);
}

void ATranspersonalHUD::DrawFearIndicator(ATranspersonalCharacter* PlayerCharacter)
{
    if (!PlayerCharacter || !Canvas || PlayerCharacter->GetFearPercentage() <= 0.0f)
    {
        return;
    }
    
    // Fear indicator in center of screen
    float CenterX = Canvas->SizeX * 0.5f;
    float CenterY = Canvas->SizeY * 0.5f;
    
    float FearIntensity = PlayerCharacter->GetFearPercentage();
    float PulseSpeed = FearIntensity * 5.0f;
    float PulseAlpha = (FMath::Sin(GetWorld()->GetTimeSeconds() * PulseSpeed) + 1.0f) * 0.5f;
    
    // Draw fear vignette effect
    float VignetteSize = 100.0f + (FearIntensity * 200.0f);
    FLinearColor VignetteColor = FearColor;
    VignetteColor.A = FearIntensity * 0.3f * PulseAlpha * HUDOpacity;
    
    FCanvasTileItem VignetteItem(FVector2D(CenterX - VignetteSize, CenterY - VignetteSize), 
                                FVector2D(VignetteSize * 2, VignetteSize * 2), VignetteColor);
    VignetteItem.BlendMode = SE_BLEND_Translucent;
    Canvas->DrawItem(VignetteItem);
    
    // Draw fear text if very high
    if (FearIntensity > 0.7f)
    {
        FCanvasTextItem FearTextItem(FVector2D(CenterX - 50.0f, CenterY + 100.0f), 
                                    FText::FromString(TEXT(\"TERRIFIED\")), 
                                    GEngine->GetMediumFont(), 
                                    FLinearColor::Red * PulseAlpha * HUDOpacity);
        Canvas->DrawItem(FearTextItem);
    }
}

void ATranspersonalHUD::DrawThreatWarning()
{
    if (!Canvas)
    {
        return;
    }
    
    // Flashing red warning
    float FlashAlpha = (FMath::Sin(GetWorld()->GetTimeSeconds() * 8.0f) + 1.0f) * 0.5f;
    FLinearColor WarningColor = FLinearColor::Red;
    WarningColor.A = FlashAlpha * 0.5f * HUDOpacity;
    
    // Draw warning overlay
    FCanvasTileItem WarningItem(FVector2D(0, 0), FVector2D(Canvas->SizeX, Canvas->SizeY), WarningColor);
    WarningItem.BlendMode = SE_BLEND_Translucent;
    Canvas->DrawItem(WarningItem);
    
    // Draw warning text
    float CenterX = Canvas->SizeX * 0.5f;
    float CenterY = Canvas->SizeY * 0.1f;
    
    FCanvasTextItem WarningTextItem(FVector2D(CenterX - 100.0f, CenterY), 
                                   FText::FromString(TEXT(\"DANGER NEARBY\")), 
                                   GEngine->GetLargeFont(), 
                                   FLinearColor::White * FlashAlpha * HUDOpacity);
    Canvas->DrawItem(WarningTextItem);
}

void ATranspersonalHUD::DrawDebugInfo(ATranspersonalCharacter* PlayerCharacter)
{
    if (!PlayerCharacter || !Canvas)
    {
        return;
    }
    
    // Debug info in top-right corner
    float StartX = Canvas->SizeX - 300.0f;
    float StartY = 50.0f;
    float LineHeight = 20.0f;
    int32 LineCount = 0;
    
    // Helper function to draw debug line
    auto DrawDebugLine = [&](const FString& Text)
    {
        FCanvasTextItem TextItem(FVector2D(StartX, StartY + (LineCount * LineHeight)), 
                                FText::FromString(Text), 
                                GEngine->GetSmallFont(), 
                                FLinearColor::Green * HUDOpacity);
        Canvas->DrawItem(TextItem);
        LineCount++;
    };
    
    // Draw debug information
    DrawDebugLine(FString::Printf(TEXT(\"Health: %.1f/%.1f\"), PlayerCharacter->Health, PlayerCharacter->MaxHealth));
    DrawDebugLine(FString::Printf(TEXT(\"Stamina: %.1f/%.1f\"), PlayerCharacter->Stamina, PlayerCharacter->MaxStamina));
    DrawDebugLine(FString::Printf(TEXT(\"Hunger: %.1f/%.1f\"), PlayerCharacter->Hunger, PlayerCharacter->MaxHunger));
    DrawDebugLine(FString::Printf(TEXT(\"Thirst: %.1f/%.1f\"), PlayerCharacter->Thirst, PlayerCharacter->MaxThirst));
    DrawDebugLine(FString::Printf(TEXT(\"Fear: %.1f/%.1f\"), PlayerCharacter->FearLevel, PlayerCharacter->MaxFearLevel));
    DrawDebugLine(FString::Printf(TEXT(\"Sneaking: %s\"), PlayerCharacter->bIsSneaking ? TEXT(\"Yes\") : TEXT(\"No\")));
    DrawDebugLine(FString::Printf(TEXT(\"Running: %s\"), PlayerCharacter->bIsRunning ? TEXT(\"Yes\") : TEXT(\"No\")));
    DrawDebugLine(FString::Printf(TEXT(\"Threat Nearby: %s\"), PlayerCharacter->bNearbyThreat ? TEXT(\"Yes\") : TEXT(\"No\")));
}

void ATranspersonalHUD::ToggleDebugInfo()
{
    bShowDebugInfo = !bShowDebugInfo;
    UE_LOG(LogTranspersonalHUD, Log, TEXT(\"Debug info toggled: %s\"), bShowDebugInfo ? TEXT(\"On\") : TEXT(\"Off\"));
}

void ATranspersonalHUD::ToggleMinimalHUD()
{
    bMinimalHUD = !bMinimalHUD;
    UE_LOG(LogTranspersonalHUD, Log, TEXT(\"Minimal HUD toggled: %s\"), bMinimalHUD ? TEXT(\"On\") : TEXT(\"Off\"));
}

void ATranspersonalHUD::SetHUDOpacity(float NewOpacity)
{
    HUDOpacity = FMath::Clamp(NewOpacity, 0.0f, 1.0f);
    UE_LOG(LogTranspersonalHUD, Log, TEXT(\"HUD opacity set to: %.2f\"), HUDOpacity);
}"