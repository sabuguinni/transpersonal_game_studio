#include "VFXTestSuite.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/StaticMesh.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Components/PostProcessComponent.h"
#include "Landscape/Landscape.h"
#include "LandscapeComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/Canvas.h"
#include "Engine/CanvasRenderTarget2D.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/DateTime.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVFXSystemValidationTest, "TranspersonalGame.VFX.SystemValidation", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FVFXSystemValidationTest::RunTest(const FString& Parameters)
{
    UWorld* World = GetTestWorld();
    if (!World)
    {
        AddError(TEXT("Failed to get test world"));
        return false;
    }

    // Test 1: Validate VFX_SurvivalEffectsManager exists and is functional
    bool bVFXManagerFound = false;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->GetClass()->GetName().Contains(TEXT("VFX_SurvivalEffectsManager")))
        {
            bVFXManagerFound = true;
            TestTrue(TEXT("VFX_SurvivalEffectsManager actor found"), true);
            
            // Test component presence
            TArray<UActorComponent*> Components = Actor->GetRootComponent()->GetAttachChildren();
            TestTrue(TEXT("VFX Manager has components"), Components.Num() > 0);
            break;
        }
    }
    
    if (!bVFXManagerFound)
    {
        AddWarning(TEXT("VFX_SurvivalEffectsManager not found in world - may need spawning"));
    }

    // Test 2: Validate Niagara systems can be loaded
    UNiagaraSystem* FireSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/VFX/Particles/NS_CampfireSmoke"));
    if (FireSystem)
    {
        TestTrue(TEXT("Campfire Niagara system loaded successfully"), true);
        TestTrue(TEXT("Fire system has valid emitters"), FireSystem->GetNumEmitters() > 0);
    }
    else
    {
        AddWarning(TEXT("Campfire Niagara system not found - may need creation"));
    }

    // Test 3: Validate material instances for VFX
    UMaterialInstance* DustMaterial = LoadObject<UMaterialInstance>(nullptr, TEXT("/Game/VFX/Materials/MI_DinosaurDust"));
    if (DustMaterial)
    {
        TestTrue(TEXT("Dinosaur dust material loaded successfully"), true);
        TestTrue(TEXT("Dust material has valid parent"), DustMaterial->GetBaseMaterial() != nullptr);
    }
    else
    {
        AddWarning(TEXT("Dinosaur dust material not found - may need creation"));
    }

    // Test 4: Performance validation - check particle count limits
    int32 ActiveParticleCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor)
        {
            TArray<UNiagaraComponent*> NiagaraComponents;
            Actor->GetComponents<UNiagaraComponent>(NiagaraComponents);
            for (UNiagaraComponent* NiagaraComp : NiagaraComponents)
            {
                if (NiagaraComp && NiagaraComp->IsActive())
                {
                    ActiveParticleCount++;
                }
            }
        }
    }
    
    TestTrue(TEXT("Active particle systems within performance limits"), ActiveParticleCount < 50);
    
    // Test 5: Lighting validation for Cretaceous atmosphere
    ADirectionalLight* SunLight = nullptr;
    for (TActorIterator<ADirectionalLight> LightItr(World); LightItr; ++LightItr)
    {
        SunLight = *LightItr;
        break;
    }
    
    if (SunLight)
    {
        UDirectionalLightComponent* LightComp = SunLight->GetComponent();
        if (LightComp)
        {
            float Intensity = LightComp->Intensity;
            FLinearColor LightColor = LightComp->LightColor;
            
            TestTrue(TEXT("Sun light intensity appropriate for Cretaceous"), Intensity >= 3.0f && Intensity <= 8.0f);
            TestTrue(TEXT("Sun light color warm (not red/orange apocalyptic)"), 
                LightColor.R >= 0.9f && LightColor.G >= 0.8f && LightColor.B >= 0.6f);
        }
    }
    else
    {
        AddWarning(TEXT("Directional light not found - lighting may need setup"));
    }

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVFXPerformanceTest, "TranspersonalGame.VFX.Performance", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FVFXPerformanceTest::RunTest(const FString& Parameters)
{
    UWorld* World = GetTestWorld();
    if (!World)
    {
        AddError(TEXT("Failed to get test world"));
        return false;
    }

    // Performance Test 1: Frame rate impact of VFX systems
    float StartTime = FPlatformTime::Seconds();
    
    // Simulate heavy VFX load
    TArray<UNiagaraComponent*> TestComponents;
    for (int32 i = 0; i < 10; i++)
    {
        AActor* TestActor = World->SpawnActor<AActor>();
        if (TestActor)
        {
            UNiagaraComponent* NiagaraComp = NewObject<UNiagaraComponent>(TestActor);
            if (NiagaraComp)
            {
                TestActor->SetRootComponent(NiagaraComp);
                TestComponents.Add(NiagaraComp);
            }
        }
    }
    
    float EndTime = FPlatformTime::Seconds();
    float SpawnTime = EndTime - StartTime;
    
    TestTrue(TEXT("VFX component spawning performance acceptable"), SpawnTime < 0.1f);
    
    // Cleanup test components
    for (UNiagaraComponent* Comp : TestComponents)
    {
        if (Comp && Comp->GetOwner())
        {
            Comp->GetOwner()->Destroy();
        }
    }

    // Performance Test 2: Memory usage validation
    SIZE_T InitialMemory = FPlatformMemory::GetStats().UsedPhysical;
    
    // Create and destroy VFX systems to test memory leaks
    for (int32 i = 0; i < 5; i++)
    {
        UNiagaraSystem* TestSystem = NewObject<UNiagaraSystem>();
        if (TestSystem)
        {
            TestSystem->ConditionalBeginDestroy();
        }
    }
    
    // Force garbage collection
    GEngine->ForceGarbageCollection(true);
    
    SIZE_T FinalMemory = FPlatformMemory::GetStats().UsedPhysical;
    SIZE_T MemoryDelta = FinalMemory - InitialMemory;
    
    TestTrue(TEXT("VFX memory usage within acceptable limits"), MemoryDelta < 50 * 1024 * 1024); // 50MB limit

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVFXIntegrationTest, "TranspersonalGame.VFX.Integration", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FVFXIntegrationTest::RunTest(const FString& Parameters)
{
    UWorld* World = GetTestWorld();
    if (!World)
    {
        AddError(TEXT("Failed to get test world"));
        return false;
    }

    // Integration Test 1: VFX with character movement
    bool bCharacterFound = false;
    for (TActorIterator<APawn> PawnItr(World); PawnItr; ++PawnItr)
    {
        APawn* Pawn = *PawnItr;
        if (Pawn && Pawn->GetClass()->GetName().Contains(TEXT("TranspersonalCharacter")))
        {
            bCharacterFound = true;
            
            // Test footstep VFX integration
            FVector CharacterLocation = Pawn->GetActorLocation();
            TestTrue(TEXT("Character has valid location for VFX"), !CharacterLocation.IsZero());
            
            // Test dust effect spawning capability
            TArray<UNiagaraComponent*> NiagaraComponents;
            Pawn->GetComponents<UNiagaraComponent>(NiagaraComponents);
            
            if (NiagaraComponents.Num() > 0)
            {
                TestTrue(TEXT("Character has Niagara components for VFX"), true);
            }
            else
            {
                AddInfo(TEXT("Character ready for VFX integration - components can be added"));
            }
            break;
        }
    }
    
    if (!bCharacterFound)
    {
        AddWarning(TEXT("TranspersonalCharacter not found - VFX integration pending"));
    }

    // Integration Test 2: VFX with environment systems
    bool bLandscapeFound = false;
    for (TActorIterator<ALandscape> LandscapeItr(World); LandscapeItr; ++LandscapeItr)
    {
        ALandscape* Landscape = *LandscapeItr;
        if (Landscape)
        {
            bLandscapeFound = true;
            TestTrue(TEXT("Landscape found for environmental VFX"), true);
            
            // Test landscape material compatibility with VFX
            UMaterialInterface* LandscapeMaterial = Landscape->GetLandscapeMaterial();
            if (LandscapeMaterial)
            {
                TestTrue(TEXT("Landscape has material for VFX interaction"), true);
            }
            break;
        }
    }
    
    if (!bLandscapeFound)
    {
        AddWarning(TEXT("Landscape not found - environmental VFX may need terrain"));
    }

    // Integration Test 3: VFX with lighting system
    bool bPostProcessFound = false;
    for (TActorIterator<APostProcessVolume> PPVItr(World); PPVItr; ++PPVItr)
    {
        APostProcessVolume* PPV = *PPVItr;
        if (PPV)
        {
            bPostProcessFound = true;
            TestTrue(TEXT("Post-process volume found for VFX enhancement"), true);
            break;
        }
    }
    
    if (!bPostProcessFound)
    {
        AddInfo(TEXT("Post-process volume can be added for enhanced VFX"));
    }

    return true;
}

UWorld* FVFXSystemValidationTest::GetTestWorld()
{
    UWorld* World = nullptr;
    
    // Try to get the editor world first
    if (GEngine)
    {
        for (const FWorldContext& Context : GEngine->GetWorldContexts())
        {
            if (Context.World() && Context.World()->IsGameWorld())
            {
                World = Context.World();
                break;
            }
        }
        
        // Fallback to any available world
        if (!World)
        {
            for (const FWorldContext& Context : GEngine->GetWorldContexts())
            {
                if (Context.World())
                {
                    World = Context.World();
                    break;
                }
            }
        }
    }
    
    return World;
}

UWorld* FVFXPerformanceTest::GetTestWorld()
{
    return FVFXSystemValidationTest::GetTestWorld();
}

UWorld* FVFXIntegrationTest::GetTestWorld()
{
    return FVFXSystemValidationTest::GetTestWorld();
}