#include "Light_PersistentAtmosphereSystem.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyAtmosphere.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "EngineUtils.h"
#include "Editor/EditorEngine.h"
#include "Editor/UnrealEdEngine.h"
#include "EditorLevelLibrary.h"

ULight_PersistentAtmosphereSystem::ULight_PersistentAtmosphereSystem()
{
    CurrentSettings = FLight_AtmosphereSettings();
}

void ULight_PersistentAtmosphereSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("PersistentAtmosphereSystem: Initializing"));
    
    // Apply Cretaceous atmosphere on startup
    GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
    {
        ApplyCretaceousAtmosphere();
    });
}

void ULight_PersistentAtmosphereSystem::Deinitialize()
{
    if (MapChangedHandle.IsValid())
    {
        // Clean up delegate if needed
        MapChangedHandle.Reset();
    }
    
    Super::Deinitialize();
}

void ULight_PersistentAtmosphereSystem::ApplyCretaceousAtmosphere()
{
    UE_LOG(LogTemp, Warning, TEXT("PersistentAtmosphereSystem: Applying Cretaceous atmosphere"));
    
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("PersistentAtmosphereSystem: No world found"));
        return;
    }

    // Remove problematic atmosphere actors first
    RemoveProblematicAtmosphereActors();
    
    // Create optimal atmosphere
    CreateOptimalAtmosphere();
    
    // Save the map to persist changes
    SaveAtmosphereSettings();
}

void ULight_PersistentAtmosphereSystem::FixAtmosphereOnMapLoad()
{
    UE_LOG(LogTemp, Warning, TEXT("PersistentAtmosphereSystem: Fixing atmosphere on map load"));
    ApplyCretaceousAtmosphere();
}

void ULight_PersistentAtmosphereSystem::SaveAtmosphereSettings()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

#if WITH_EDITOR
    // Save the current map to persist atmosphere changes
    if (GEditor && GEditor->GetEditorWorldContext().World() == World)
    {
        FString MapPath = TEXT("/Game/Maps/MinPlayableMap");
        bool bSaveResult = UEditorLoadingAndSavingUtils::SaveMap(World, MapPath);
        UE_LOG(LogTemp, Warning, TEXT("PersistentAtmosphereSystem: Map saved with result: %s"), 
               bSaveResult ? TEXT("SUCCESS") : TEXT("FAILED"));
    }
#endif
}

void ULight_PersistentAtmosphereSystem::LoadAtmosphereSettings()
{
    // Load settings from saved game or config if needed
    // For now, use default Cretaceous settings
    CurrentSettings = FLight_AtmosphereSettings();
}

void ULight_PersistentAtmosphereSystem::RemoveProblematicAtmosphereActors()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    TArray<AActor*> ActorsToRemove;
    
    // Find problematic atmosphere actors
    for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (!Actor)
        {
            continue;
        }

        FString ClassName = Actor->GetClass()->GetName();
        
        // Remove old sky atmosphere or atmospheric fog actors that might cause orange tint
        if (ClassName.Contains(TEXT("SkyAtmosphere")) || 
            ClassName.Contains(TEXT("AtmosphericFog")) ||
            ClassName.Contains(TEXT("BP_Sky")))
        {
            ActorsToRemove.Add(Actor);
        }
    }

    // Remove problematic actors
    for (AActor* Actor : ActorsToRemove)
    {
        if (Actor && IsValid(Actor))
        {
            UE_LOG(LogTemp, Warning, TEXT("PersistentAtmosphereSystem: Removing problematic actor: %s"), 
                   *Actor->GetClass()->GetName());
            Actor->Destroy();
        }
    }
}

void ULight_PersistentAtmosphereSystem::CreateOptimalAtmosphere()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    FindOrCreateAtmosphereActors();
    
    // Configure each component with optimal Cretaceous settings
    if (MainSunLight.IsValid())
    {
        ConfigureDirectionalLight(MainSunLight.Get());
    }
    
    if (MainFog.IsValid())
    {
        ConfigureHeightFog(MainFog.Get());
    }
    
    if (MainAtmosphere.IsValid())
    {
        ConfigureSkyAtmosphere(MainAtmosphere.Get());
    }
}

void ULight_PersistentAtmosphereSystem::FindOrCreateAtmosphereActors()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Find or create directional light
    for (TActorIterator<ADirectionalLight> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        MainSunLight = *ActorIterator;
        break;
    }
    
    if (!MainSunLight.IsValid())
    {
        MainSunLight = World->SpawnActor<ADirectionalLight>();
        if (MainSunLight.IsValid())
        {
            MainSunLight->SetActorLabel(TEXT("CretaceousSun"));
        }
    }

    // Find or create exponential height fog
    for (TActorIterator<AExponentialHeightFog> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        MainFog = *ActorIterator;
        break;
    }
    
    if (!MainFog.IsValid())
    {
        MainFog = World->SpawnActor<AExponentialHeightFog>();
        if (MainFog.IsValid())
        {
            MainFog->SetActorLabel(TEXT("CretaceousFog"));
        }
    }

    // Find or create sky atmosphere
    for (TActorIterator<ASkyAtmosphere> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        MainAtmosphere = *ActorIterator;
        break;
    }
    
    if (!MainAtmosphere.IsValid())
    {
        MainAtmosphere = World->SpawnActor<ASkyAtmosphere>();
        if (MainAtmosphere.IsValid())
        {
            MainAtmosphere->SetActorLabel(TEXT("CretaceousAtmosphere"));
        }
    }
}

void ULight_PersistentAtmosphereSystem::ConfigureDirectionalLight(ADirectionalLight* Light)
{
    if (!Light)
    {
        return;
    }

    UDirectionalLightComponent* LightComp = Light->GetLightComponent();
    if (!LightComp)
    {
        return;
    }

    // Set Cretaceous period lighting
    LightComp->SetIntensity(CurrentSettings.SunIntensity);
    LightComp->SetLightColor(CurrentSettings.SunColor);
    LightComp->SetCastShadows(true);
    LightComp->SetCastVolumetricShadow(true);
    
    // Set sun angle for mid-day Cretaceous lighting
    Light->SetActorRotation(FRotator(-45.0f, 30.0f, 0.0f));
    
    UE_LOG(LogTemp, Warning, TEXT("PersistentAtmosphereSystem: Configured directional light"));
}

void ULight_PersistentAtmosphereSystem::ConfigureHeightFog(AExponentialHeightFog* Fog)
{
    if (!Fog)
    {
        return;
    }

    UExponentialHeightFogComponent* FogComp = Fog->GetComponent();
    if (!FogComp)
    {
        return;
    }

    // Set subtle atmospheric fog
    FogComp->SetFogDensity(CurrentSettings.FogDensity);
    FogComp->SetFogHeightFalloff(CurrentSettings.FogHeightFalloff);
    FogComp->SetFogInscatteringColor(CurrentSettings.FogInscatteringColor);
    FogComp->SetVolumetricFog(CurrentSettings.bEnableVolumetricFog);
    FogComp->SetVolumetricFogScatteringDistribution(0.2f);
    
    UE_LOG(LogTemp, Warning, TEXT("PersistentAtmosphereSystem: Configured height fog"));
}

void ULight_PersistentAtmosphereSystem::ConfigureSkyAtmosphere(ASkyAtmosphere* Atmosphere)
{
    if (!Atmosphere)
    {
        return;
    }

    USkyAtmosphereComponent* AtmComp = Atmosphere->GetSkyAtmosphereComponent();
    if (!AtmComp)
    {
        return;
    }

    // Set realistic Cretaceous atmosphere
    AtmComp->SetAtmosphereHeight(CurrentSettings.AtmosphereHeight);
    AtmComp->SetRayleighScattering(FLinearColor(CurrentSettings.RayleighScattering, 
                                               CurrentSettings.RayleighScattering * 1.2f, 
                                               CurrentSettings.RayleighScattering * 2.8f));
    AtmComp->SetMieScattering(FLinearColor(0.003996f, 0.003996f, 0.003996f));
    
    UE_LOG(LogTemp, Warning, TEXT("PersistentAtmosphereSystem: Configured sky atmosphere"));
}

void ULight_PersistentAtmosphereSystem::SetAtmosphereSettings(const FLight_AtmosphereSettings& Settings)
{
    CurrentSettings = Settings;
    ApplyCretaceousAtmosphere();
}

void ULight_PersistentAtmosphereSystem::OnMapChanged()
{
    // Reapply atmosphere when map changes
    GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
    {
        FixAtmosphereOnMapLoad();
    });
}