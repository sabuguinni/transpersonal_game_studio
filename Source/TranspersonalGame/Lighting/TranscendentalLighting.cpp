// TranscendentalLighting.cpp
// Implementação do sistema de iluminação transcendental
#include "TranscendentalLighting.h"
#include "Engine/World.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"

UTranscendentalLighting::UTranscendentalLighting()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostUpdateWork;

    // Configurações padrão para cada estado de consciência
    FLightingConfiguration DormantConfig;
    DormantConfig.BaseColor = FLinearColor(0.8f, 0.7f, 0.6f, 1.0f); // Luz quente e suave
    DormantConfig.Intensity = 0.3f;
    DormantConfig.Temperature = 3200.0f;
    DormantConfig.Pattern = ELightPattern::Breathing;
    DormantConfig.AnimationSpeed = 0.5f;
    DormantConfig.VolumetricScattering = 0.2f;
    LightingStates.Add(ELightingState::Dormant, DormantConfig);

    FLightingConfiguration AwakeningConfig;
    AwakeningConfig.BaseColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f); // Dourado suave
    AwakeningConfig.Intensity = 0.6f;
    AwakeningConfig.Temperature = 4500.0f;
    AwakeningConfig.Pattern = ELightPattern::Pulsing;
    AwakeningConfig.AnimationSpeed = 1.0f;
    AwakeningConfig.VolumetricScattering = 0.4f;
    LightingStates.Add(ELightingState::Awakening, AwakeningConfig);

    FLightingConfiguration TranscendentConfig;
    TranscendentConfig.BaseColor = FLinearColor(0.9f, 0.95f, 1.0f, 1.0f); // Azul cristalino
    TranscendentConfig.Intensity = 1.2f;
    TranscendentConfig.Temperature = 6500.0f;
    TranscendentConfig.Pattern = ELightPattern::Flowing;
    TranscendentConfig.AnimationSpeed = 1.5f;
    TranscendentConfig.VolumetricScattering = 0.7f;
    LightingStates.Add(ELightingState::Transcendent, TranscendentConfig);

    FLightingConfiguration UnityConfig;
    UnityConfig.BaseColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f); // Luz pura
    UnityConfig.Intensity = 2.0f;
    UnityConfig.Temperature = 6500.0f;
    UnityConfig.Pattern = ELightPattern::Spiraling;
    UnityConfig.AnimationSpeed = 2.0f;
    UnityConfig.VolumetricScattering = 1.0f;
    LightingStates.Add(ELightingState::Unity, UnityConfig);

    FLightingConfiguration VoidConfig;
    VoidConfig.BaseColor = FLinearColor(0.1f, 0.05f, 0.2f, 1.0f); // Violeta profundo
    VoidConfig.Intensity = 0.1f;
    VoidConfig.Temperature = 8000.0f;
    VoidConfig.Pattern = ELightPattern::Static;
    VoidConfig.AnimationSpeed = 0.1f;
    VoidConfig.VolumetricScattering = 0.1f;
    LightingStates.Add(ELightingState::Void, VoidConfig);
}

void UTranscendentalLighting::BeginPlay()
{
    Super::BeginPlay();
    
    // Encontrar e registrar luzes existentes no nível
    UWorld* World = GetWorld();
    if (World)
    {
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor)
            {
                TArray<ULightComponent*> LightComponents;
                Actor->GetComponents<ULightComponent>(LightComponents);
                
                for (ULightComponent* Light : LightComponents)
                {
                    if (Light && Light->GetName().Contains(TEXT("Transcendental")))
                    {
                        RegisterLight(Light);
                    }
                }
            }
        }
    }

    // Aplicar configuração inicial
    if (LightingStates.Contains(CurrentState))
    {
        ApplyLightingConfiguration(LightingStates[CurrentState]);
    }
}

void UTranscendentalLighting::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    AnimationTime += DeltaTime;

    if (bIsTransitioning)
    {
        UpdateTransition(DeltaTime);
    }
    else
    {
        UpdateLightingAnimation(DeltaTime);
    }
}

void UTranscendentalLighting::SetLightingState(ELightingState NewState)
{
    if (NewState == CurrentState && !bIsTransitioning)
    {
        return;
    }

    PreviousState = CurrentState;
    TargetState = NewState;
    bIsTransitioning = true;
    TransitionProgress = 0.0f;

    UE_LOG(LogTemp, Log, TEXT("Transitioning lighting from %d to %d"), (int32)PreviousState, (int32)TargetState);
}

void UTranscendentalLighting::RegisterLight(ULightComponent* LightComponent)
{
    if (LightComponent && !ManagedLights.Contains(LightComponent))
    {
        ManagedLights.Add(LightComponent);
        
        // Configurar propriedades especiais para iluminação transcendental
        LightComponent->SetCastVolumetricShadow(true);
        LightComponent->SetVolumetricScatteringIntensity(0.5f);
        
        UE_LOG(LogTemp, Log, TEXT("Registered light: %s"), *LightComponent->GetName());
    }
}

void UTranscendentalLighting::UnregisterLight(ULightComponent* LightComponent)
{
    if (LightComponent)
    {
        ManagedLights.Remove(LightComponent);
        UE_LOG(LogTemp, Log, TEXT("Unregistered light: %s"), *LightComponent->GetName());
    }
}

void UTranscendentalLighting::SetGlobalIntensityMultiplier(float Multiplier)
{
    GlobalIntensityMultiplier = FMath::Clamp(Multiplier, 0.0f, 10.0f);
    
    // Aplicar imediatamente a todas as luzes
    if (LightingStates.Contains(CurrentState))
    {
        ApplyLightingConfiguration(LightingStates[CurrentState]);
    }
}

void UTranscendentalLighting::TriggerLightingEvent(const FString& EventName)
{
    if (EventName == TEXT("DivinePresence"))
    {
        // Criar efeito de presença divina
        for (ULightComponent* Light : ManagedLights)
        {
            if (Light)
            {
                float OriginalIntensity = Light->Intensity;
                Light->SetIntensity(OriginalIntensity * 3.0f);
                
                // Programar retorno ao normal após 2 segundos
                FTimerHandle TimerHandle;
                GetWorld()->GetTimerManager().SetTimer(TimerHandle, [Light, OriginalIntensity]()
                {
                    if (Light)
                    {
                        Light->SetIntensity(OriginalIntensity);
                    }
                }, 2.0f, false);
            }
        }
    }
    else if (EventName == TEXT("EnergyWave"))
    {
        // Criar onda de energia que percorre todas as luzes
        float Delay = 0.0f;
        for (ULightComponent* Light : ManagedLights)
        {
            if (Light)
            {
                FTimerHandle TimerHandle;
                GetWorld()->GetTimerManager().SetTimer(TimerHandle, [Light]()
                {
                    if (Light)
                    {
                        Light->SetLightColor(FLinearColor::Blue);
                        
                        FTimerHandle ResetTimer;
                        Light->GetWorld()->GetTimerManager().SetTimer(ResetTimer, [Light]()
                        {
                            if (Light)
                            {
                                Light->SetLightColor(FLinearColor::White);
                            }
                        }, 0.5f, false);
                    }
                }, Delay, false);
                
                Delay += 0.2f;
            }
        }
    }
}

void UTranscendentalLighting::CreateDivineRay(FVector StartLocation, FVector EndLocation, float Duration)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Criar luz spot para simular raio divino
    ASpotLight* DivineRay = World->SpawnActor<ASpotLight>(StartLocation, FRotator::ZeroRotator);
    if (DivineRay)
    {
        USpotLightComponent* SpotLight = DivineRay->GetSpotLightComponent();
        if (SpotLight)
        {
            SpotLight->SetLightColor(FLinearColor(1.0f, 0.95f, 0.8f));
            SpotLight->SetIntensity(5000.0f);
            SpotLight->SetInnerConeAngle(5.0f);
            SpotLight->SetOuterConeAngle(15.0f);
            SpotLight->SetAttenuationRadius(FVector::Dist(StartLocation, EndLocation));
            SpotLight->SetCastVolumetricShadow(true);
            SpotLight->SetVolumetricScatteringIntensity(2.0f);

            // Orientar em direção ao ponto final
            FVector Direction = (EndLocation - StartLocation).GetSafeNormal();
            DivineRay->SetActorRotation(Direction.Rotation());

            // Destruir após a duração especificada
            FTimerHandle DestroyTimer;
            World->GetTimerManager().SetTimer(DestroyTimer, [DivineRay]()
            {
                if (DivineRay)
                {
                    DivineRay->Destroy();
                }
            }, Duration, false);
        }
    }
}

void UTranscendentalLighting::CreateAuraEffect(AActor* TargetActor, FLinearColor AuraColor, float Radius)
{
    if (!TargetActor)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Criar luz point para simular aura
    APointLight* AuraLight = World->SpawnActor<APointLight>(TargetActor->GetActorLocation(), FRotator::ZeroRotator);
    if (AuraLight)
    {
        UPointLightComponent* PointLight = AuraLight->GetPointLightComponent();
        if (PointLight)
        {
            PointLight->SetLightColor(AuraColor);
            PointLight->SetIntensity(1000.0f);
            PointLight->SetAttenuationRadius(Radius);
            PointLight->SetCastVolumetricShadow(false);
            PointLight->SetVolumetricScatteringIntensity(1.5f);

            // Anexar ao ator alvo
            AuraLight->AttachToActor(TargetActor, FAttachmentTransformRules::KeepWorldTransform);
        }
    }
}

void UTranscendentalLighting::CreateEnergyVortex(FVector Location, float Radius, float Duration)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Criar múltiplas luzes em espiral para simular vórtice
    int32 NumLights = 8;
    float AngleStep = 360.0f / NumLights;
    
    TArray<APointLight*> VortexLights;
    
    for (int32 i = 0; i < NumLights; i++)
    {
        float Angle = i * AngleStep;
        float RadianAngle = FMath::DegreesToRadians(Angle);
        
        FVector LightLocation = Location + FVector(
            FMath::Cos(RadianAngle) * Radius * 0.5f,
            FMath::Sin(RadianAngle) * Radius * 0.5f,
            FMath::Sin(RadianAngle * 2) * 50.0f
        );
        
        APointLight* VortexLight = World->SpawnActor<APointLight>(LightLocation, FRotator::ZeroRotator);
        if (VortexLight)
        {
            UPointLightComponent* PointLight = VortexLight->GetPointLightComponent();
            if (PointLight)
            {
                PointLight->SetLightColor(FLinearColor::Blue);
                PointLight->SetIntensity(2000.0f);
                PointLight->SetAttenuationRadius(Radius * 0.3f);
                PointLight->SetCastVolumetricShadow(true);
                PointLight->SetVolumetricScatteringIntensity(2.0f);
                
                VortexLights.Add(VortexLight);
            }
        }
    }

    // Animar o vórtice
    FTimerHandle AnimationTimer;
    float AnimationTime = 0.0f;
    
    World->GetTimerManager().SetTimer(AnimationTimer, [VortexLights, Location, Radius, &AnimationTime]() mutable
    {
        AnimationTime += 0.1f;
        
        for (int32 i = 0; i < VortexLights.Num(); i++)
        {
            if (VortexLights[i])
            {
                float Angle = (i * 45.0f) + (AnimationTime * 180.0f);
                float RadianAngle = FMath::DegreesToRadians(Angle);
                
                FVector NewLocation = Location + FVector(
                    FMath::Cos(RadianAngle) * Radius * 0.5f,
                    FMath::Sin(RadianAngle) * Radius * 0.5f,
                    FMath::Sin(RadianAngle * 2 + AnimationTime) * 50.0f
                );
                
                VortexLights[i]->SetActorLocation(NewLocation);
            }
        }
    }, 0.1f, true);

    // Destruir após a duração
    FTimerHandle DestroyTimer;
    World->GetTimerManager().SetTimer(DestroyTimer, [VortexLights, AnimationTimer, World]()
    {
        World->GetTimerManager().ClearTimer(AnimationTimer);
        
        for (APointLight* Light : VortexLights)
        {
            if (Light)
            {
                Light->Destroy();
            }
        }
    }, Duration, false);
}

void UTranscendentalLighting::UpdateLightingAnimation(float DeltaTime)
{
    if (!LightingStates.Contains(CurrentState))
    {
        return;
    }

    const FLightingConfiguration& Config = LightingStates[CurrentState];
    
    for (ULightComponent* Light : ManagedLights)
    {
        if (Light)
        {
            ApplyLightPattern(Light, Config.Pattern, AnimationTime * Config.AnimationSpeed);
        }
    }
}

void UTranscendentalLighting::ApplyLightingConfiguration(const FLightingConfiguration& Config, float Alpha)
{
    for (ULightComponent* Light : ManagedLights)
    {
        if (Light)
        {
            // Aplicar cor
            FLinearColor CurrentColor = Light->GetLightColor();
            FLinearColor TargetColor = Config.BaseColor;
            Light->SetLightColor(FMath::Lerp(CurrentColor, TargetColor, Alpha));

            // Aplicar intensidade
            float CurrentIntensity = Light->Intensity;
            float TargetIntensity = Config.Intensity * GlobalIntensityMultiplier;
            Light->SetIntensity(FMath::Lerp(CurrentIntensity, TargetIntensity, Alpha));

            // Aplicar temperatura
            Light->SetTemperature(Config.Temperature);

            // Aplicar configurações volumétricas
            Light->SetVolumetricScatteringIntensity(Config.VolumetricScattering);
            Light->SetCastVolumetricShadow(Config.bCastVolumetricShadow);
        }
    }
}

void UTranscendentalLighting::UpdateTransition(float DeltaTime)
{
    TransitionProgress += DeltaTime / TransitionDuration;
    
    if (TransitionProgress >= 1.0f)
    {
        TransitionProgress = 1.0f;
        bIsTransitioning = false;
        CurrentState = TargetState;
    }

    // Aplicar curva de transição se disponível
    float Alpha = TransitionProgress;
    if (TransitionCurve)
    {
        Alpha = TransitionCurve->GetFloatValue(TransitionProgress);
    }

    // Interpolar entre configurações
    if (LightingStates.Contains(PreviousState) && LightingStates.Contains(TargetState))
    {
        const FLightingConfiguration& PrevConfig = LightingStates[PreviousState];
        const FLightingConfiguration& NextConfig = LightingStates[TargetState];

        for (ULightComponent* Light : ManagedLights)
        {
            if (Light)
            {
                // Interpolar cor
                FLinearColor InterpolatedColor = InterpolateLightColor(PrevConfig.BaseColor, NextConfig.BaseColor, Alpha);
                Light->SetLightColor(InterpolatedColor);

                // Interpolar intensidade
                float InterpolatedIntensity = FMath::Lerp(PrevConfig.Intensity, NextConfig.Intensity, Alpha) * GlobalIntensityMultiplier;
                Light->SetIntensity(InterpolatedIntensity);

                // Interpolar temperatura
                float InterpolatedTemp = FMath::Lerp(PrevConfig.Temperature, NextConfig.Temperature, Alpha);
                Light->SetTemperature(InterpolatedTemp);

                // Interpolar volumétrico
                float InterpolatedScattering = FMath::Lerp(PrevConfig.VolumetricScattering, NextConfig.VolumetricScattering, Alpha);
                Light->SetVolumetricScatteringIntensity(InterpolatedScattering);
            }
        }
    }
}

FLinearColor UTranscendentalLighting::InterpolateLightColor(const FLinearColor& ColorA, const FLinearColor& ColorB, float Alpha)
{
    return FLinearColor(
        FMath::Lerp(ColorA.R, ColorB.R, Alpha),
        FMath::Lerp(ColorA.G, ColorB.G, Alpha),
        FMath::Lerp(ColorA.B, ColorB.B, Alpha),
        FMath::Lerp(ColorA.A, ColorB.A, Alpha)
    );
}

void UTranscendentalLighting::ApplyLightPattern(ULightComponent* Light, ELightPattern Pattern, float Time)
{
    if (!Light)
    {
        return;
    }

    float BaseIntensity = Light->Intensity;
    
    switch (Pattern)
    {
        case ELightPattern::Static:
            // Sem animação
            break;
            
        case ELightPattern::Breathing:
        {
            float BreathingMultiplier = 0.5f + 0.5f * FMath::Sin(Time * 2.0f);
            Light->SetIntensity(BaseIntensity * BreathingMultiplier);
            break;
        }
        
        case ELightPattern::Pulsing:
        {
            float PulseMultiplier = 0.3f + 0.7f * (0.5f + 0.5f * FMath::Sin(Time * 4.0f));
            Light->SetIntensity(BaseIntensity * PulseMultiplier);
            break;
        }
        
        case ELightPattern::Flowing:
        {
            float FlowMultiplier = 0.6f + 0.4f * FMath::Sin(Time * 1.5f + Light->GetComponentLocation().X * 0.01f);
            Light->SetIntensity(BaseIntensity * FlowMultiplier);
            break;
        }
        
        case ELightPattern::Spiraling:
        {
            float SpiralMultiplier = 0.4f + 0.6f * FMath::Sin(Time * 3.0f + Light->GetComponentLocation().Y * 0.01f);
            Light->SetIntensity(BaseIntensity * SpiralMultiplier);
            
            // Modificar cor ligeiramente
            FLinearColor BaseColor = Light->GetLightColor();
            float ColorShift = 0.1f * FMath::Sin(Time * 2.0f);
            Light->SetLightColor(FLinearColor(
                FMath::Clamp(BaseColor.R + ColorShift, 0.0f, 1.0f),
                FMath::Clamp(BaseColor.G + ColorShift * 0.5f, 0.0f, 1.0f),
                FMath::Clamp(BaseColor.B + ColorShift * 0.8f, 0.0f, 1.0f),
                BaseColor.A
            ));
            break;
        }
        
        case ELightPattern::Cascading:
        {
            float CascadeTime = Time + Light->GetComponentLocation().Z * 0.005f;
            float CascadeMultiplier = 0.2f + 0.8f * FMath::Max(0.0f, FMath::Sin(CascadeTime * 2.5f));
            Light->SetIntensity(BaseIntensity * CascadeMultiplier);
            break;
        }
    }
}