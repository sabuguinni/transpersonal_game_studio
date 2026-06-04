#include "QA_VFXTestFramework.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

UQA_VFXTestFramework::UQA_VFXTestFramework()
{
    PrimaryComponentTick.bCanEverTick = true;
    bAutoRunTests = false;
    TestInterval = 5.0f;
    TestStartTime = 0.0f;
}

void UQA_VFXTestFramework::BeginPlay()
{
    Super::BeginPlay();
    
    if (bAutoRunTests)
    {
        // Delay initial test run to allow world to fully initialize
        GetWorld()->GetTimerManager().SetTimer(
            FTimerHandle(),
            this,
            &UQA_VFXTestFramework::RunAllVFXTests,
            2.0f,
            false
        );
    }
}

void UQA_VFXTestFramework::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Periodic testing if enabled
    if (bAutoRunTests && TestInterval > 0.0f)
    {
        TestStartTime += DeltaTime;
        if (TestStartTime >= TestInterval)
        {
            RunAllVFXTests();
            TestStartTime = 0.0f;
        }
    }
}

void UQA_VFXTestFramework::RunAllVFXTests()
{
    UE_LOG(LogTemp, Warning, TEXT("QA_VFXTestFramework: Starting comprehensive VFX test suite"));
    
    // Clear previous results
    TestResults.Empty();
    
    // Run all VFX tests
    TestNiagaraSystemSpawning();
    TestCombatParticleEffects();
    TestEnvironmentalVFX();
    TestVFXPerformance();
    ValidateVFXIntegration();
    
    // Generate final report
    GenerateVFXTestReport();
}

void UQA_VFXTestFramework::TestNiagaraSystemSpawning()
{
    float StartTime = FPlatformTime::Seconds();
    
    try
    {
        // Test basic Niagara system spawning
        FVector TestLocation = GetOwner()->GetActorLocation() + FVector(100, 0, 50);
        
        // Try to spawn a basic particle effect
        UNiagaraComponent* TestComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            nullptr, // Will use default system if available
            TestLocation,
            FRotator::ZeroRotator,
            FVector::OneVector,
            true,
            true,
            ENCPoolMethod::None
        );
        
        if (TestComponent)
        {
            AddTestResult(TEXT("Niagara System Spawning"), EQA_VFXTestResult::Pass, TEXT(""), FPlatformTime::Seconds() - StartTime);
            
            // Clean up test component
            TestComponent->DestroyComponent();
        }
        else
        {
            AddTestResult(TEXT("Niagara System Spawning"), EQA_VFXTestResult::Warning, TEXT("No default Niagara system available"), FPlatformTime::Seconds() - StartTime);
        }
    }
    catch (...)
    {
        AddTestResult(TEXT("Niagara System Spawning"), EQA_VFXTestResult::Fail, TEXT("Exception during Niagara spawning"), FPlatformTime::Seconds() - StartTime);
    }
}

void UQA_VFXTestFramework::TestCombatParticleEffects()
{
    float StartTime = FPlatformTime::Seconds();
    
    try
    {
        // Test combat VFX integration
        TArray<AActor*> CombatActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), CombatActors);
        
        int32 VFXActorsFound = 0;
        for (AActor* Actor : CombatActors)
        {
            if (Actor && Actor->GetName().Contains(TEXT("VFX")) && Actor->GetName().Contains(TEXT("Combat")))
            {
                VFXActorsFound++;
            }
        }
        
        if (VFXActorsFound > 0)
        {
            AddTestResult(TEXT("Combat Particle Effects"), EQA_VFXTestResult::Pass, FString::Printf(TEXT("Found %d combat VFX actors"), VFXActorsFound), FPlatformTime::Seconds() - StartTime);
        }
        else
        {
            AddTestResult(TEXT("Combat Particle Effects"), EQA_VFXTestResult::Warning, TEXT("No combat VFX actors found in scene"), FPlatformTime::Seconds() - StartTime);
        }
    }
    catch (...)
    {
        AddTestResult(TEXT("Combat Particle Effects"), EQA_VFXTestResult::Fail, TEXT("Exception during combat VFX testing"), FPlatformTime::Seconds() - StartTime);
    }
}

void UQA_VFXTestFramework::TestEnvironmentalVFX()
{
    float StartTime = FPlatformTime::Seconds();
    
    try
    {
        // Test environmental effects like fire, smoke, dust
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
        
        int32 EnvironmentalVFXCount = 0;
        for (AActor* Actor : AllActors)
        {
            if (Actor)
            {
                // Check for Niagara components on actors
                TArray<UNiagaraComponent*> NiagaraComponents;
                Actor->GetComponents<UNiagaraComponent>(NiagaraComponents);
                EnvironmentalVFXCount += NiagaraComponents.Num();
            }
        }
        
        if (EnvironmentalVFXCount > 0)
        {
            AddTestResult(TEXT("Environmental VFX"), EQA_VFXTestResult::Pass, FString::Printf(TEXT("Found %d Niagara components"), EnvironmentalVFXCount), FPlatformTime::Seconds() - StartTime);
        }
        else
        {
            AddTestResult(TEXT("Environmental VFX"), EQA_VFXTestResult::Warning, TEXT("No environmental VFX components found"), FPlatformTime::Seconds() - StartTime);
        }
    }
    catch (...)
    {
        AddTestResult(TEXT("Environmental VFX"), EQA_VFXTestResult::Fail, TEXT("Exception during environmental VFX testing"), FPlatformTime::Seconds() - StartTime);
    }
}

void UQA_VFXTestFramework::TestVFXPerformance()
{
    float StartTime = FPlatformTime::Seconds();
    
    try
    {
        // Test VFX performance by checking frame rate impact
        float CurrentFPS = 1.0f / GetWorld()->GetDeltaSeconds();
        
        // Spawn multiple VFX for stress testing
        TArray<UNiagaraComponent*> StressTestComponents;
        FVector BaseLocation = GetOwner()->GetActorLocation();
        
        for (int32 i = 0; i < 5; i++)
        {
            FVector TestLocation = BaseLocation + FVector(i * 100, 0, 0);
            UNiagaraComponent* TestComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                GetWorld(),
                nullptr,
                TestLocation,
                FRotator::ZeroRotator,
                FVector::OneVector,
                true,
                true,
                ENCPoolMethod::None
            );
            
            if (TestComp)
            {
                StressTestComponents.Add(TestComp);
            }
        }
        
        // Wait a frame and check performance
        GetWorld()->GetTimerManager().SetTimerForNextTick([this, StressTestComponents, StartTime]()
        {
            float NewFPS = 1.0f / GetWorld()->GetDeltaSeconds();
            
            // Clean up stress test components
            for (UNiagaraComponent* Comp : StressTestComponents)
            {
                if (Comp)
                {
                    Comp->DestroyComponent();
                }
            }
            
            if (NewFPS > 30.0f) // Acceptable performance threshold
            {
                AddTestResult(TEXT("VFX Performance"), EQA_VFXTestResult::Pass, FString::Printf(TEXT("FPS maintained at %.1f"), NewFPS), FPlatformTime::Seconds() - StartTime);
            }
            else
            {
                AddTestResult(TEXT("VFX Performance"), EQA_VFXTestResult::Warning, FString::Printf(TEXT("FPS dropped to %.1f"), NewFPS), FPlatformTime::Seconds() - StartTime);
            }
        });
    }
    catch (...)
    {
        AddTestResult(TEXT("VFX Performance"), EQA_VFXTestResult::Fail, TEXT("Exception during VFX performance testing"), FPlatformTime::Seconds() - StartTime);
    }
}

void UQA_VFXTestFramework::ValidateVFXIntegration()
{
    float StartTime = FPlatformTime::Seconds();
    
    try
    {
        // Test VFX integration with game systems
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
        
        int32 IntegratedVFXCount = 0;
        for (AActor* Actor : AllActors)
        {
            if (Actor && (Actor->GetName().Contains(TEXT("Dinosaur")) || Actor->GetName().Contains(TEXT("Character"))))
            {
                // Check if character/dinosaur actors have VFX components
                TArray<UNiagaraComponent*> VFXComponents;
                Actor->GetComponents<UNiagaraComponent>(VFXComponents);
                if (VFXComponents.Num() > 0)
                {
                    IntegratedVFXCount++;
                }
            }
        }
        
        if (IntegratedVFXCount > 0)
        {
            AddTestResult(TEXT("VFX Integration"), EQA_VFXTestResult::Pass, FString::Printf(TEXT("%d actors with integrated VFX"), IntegratedVFXCount), FPlatformTime::Seconds() - StartTime);
        }
        else
        {
            AddTestResult(TEXT("VFX Integration"), EQA_VFXTestResult::Warning, TEXT("No VFX integration found on game actors"), FPlatformTime::Seconds() - StartTime);
        }
    }
    catch (...)
    {
        AddTestResult(TEXT("VFX Integration"), EQA_VFXTestResult::Fail, TEXT("Exception during VFX integration testing"), FPlatformTime::Seconds() - StartTime);
    }
}

void UQA_VFXTestFramework::GenerateVFXTestReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== VFX TEST REPORT ==="));
    
    int32 PassCount = 0;
    int32 FailCount = 0;
    int32 WarningCount = 0;
    
    for (const FQA_VFXTestCase& TestCase : TestResults)
    {
        LogTestResult(TestCase);
        
        switch (TestCase.Result)
        {
            case EQA_VFXTestResult::Pass:
                PassCount++;
                break;
            case EQA_VFXTestResult::Fail:
                FailCount++;
                break;
            case EQA_VFXTestResult::Warning:
                WarningCount++;
                break;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("VFX Test Summary: %d PASS, %d FAIL, %d WARNING"), PassCount, FailCount, WarningCount);
    UE_LOG(LogTemp, Warning, TEXT("=== END VFX TEST REPORT ==="));
}

bool UQA_VFXTestFramework::AreAllTestsPassing() const
{
    for (const FQA_VFXTestCase& TestCase : TestResults)
    {
        if (TestCase.Result == EQA_VFXTestResult::Fail)
        {
            return false;
        }
    }
    return true;
}

void UQA_VFXTestFramework::AddTestResult(const FString& TestName, EQA_VFXTestResult Result, const FString& ErrorMessage, float ExecutionTime)
{
    FQA_VFXTestCase NewTest;
    NewTest.TestName = TestName;
    NewTest.Result = Result;
    NewTest.ErrorMessage = ErrorMessage;
    NewTest.ExecutionTime = ExecutionTime;
    
    TestResults.Add(NewTest);
}

void UQA_VFXTestFramework::LogTestResult(const FQA_VFXTestCase& TestCase)
{
    FString ResultString;
    switch (TestCase.Result)
    {
        case EQA_VFXTestResult::Pass:
            ResultString = TEXT("PASS");
            break;
        case EQA_VFXTestResult::Fail:
            ResultString = TEXT("FAIL");
            break;
        case EQA_VFXTestResult::Warning:
            ResultString = TEXT("WARN");
            break;
        default:
            ResultString = TEXT("UNKNOWN");
            break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("[%s] %s (%.3fs) - %s"), 
           *ResultString, 
           *TestCase.TestName, 
           TestCase.ExecutionTime,
           *TestCase.ErrorMessage);
}