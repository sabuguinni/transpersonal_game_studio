#include "MS_DynamicMusicSystem.h"
#include "MetasoundExecutableOperator.h"
#include "MetasoundPrimitives.h"
#include "MetasoundStandardNodesCategories.h"
#include "MetasoundVertex.h"

#define LOCTEXT_NAMESPACE "DynamicMusicSystem"

namespace Metasound
{
    // Dynamic Music System Node - Controla música adaptativa baseada no estado emocional
    class FDynamicMusicSystemOperator : public TExecutableOperator<FDynamicMusicSystemOperator>
    {
    public:
        static const FVertexInterface& GetVertexInterface()
        {
            static const FVertexInterface Interface(
                FInputVertexInterface(
                    TInputDataVertex<float>("ThreatLevel", LOCTEXT("ThreatLevelDesc", "Nível de ameaça (0-1)")),
                    TInputDataVertex<float>("PlayerHeartRate", LOCTEXT("HeartRateDesc", "Batimento cardíaco simulado")),
                    TInputDataVertex<bool>("IsDinosaur", LOCTEXT("IsDinosaurDesc", "Dinossauro próximo detectado")),
                    TInputDataVertex<float>("TimeOfDay", LOCTEXT("TimeOfDayDesc", "Hora do dia (0-24)")),
                    TInputDataVertex<bool>("IsHiding", LOCTEXT("IsHidingDesc", "Jogador escondido"))
                ),
                FOutputVertexInterface(
                    TOutputDataVertex<FAudioBuffer>("MusicOut", LOCTEXT("MusicOutDesc", "Saída musical adaptativa")),
                    TOutputDataVertex<float>("IntensityLevel", LOCTEXT("IntensityDesc", "Nível de intensidade atual")),
                    TOutputDataVertex<bool>("ShouldTriggerStinger", LOCTEXT("StingerDesc", "Trigger para stinger de tensão"))
                )
            );
            return Interface;
        }

        static const FNodeClassMetadata& GetNodeInfo()
        {
            auto CreateNodeClassMetadata = []() -> FNodeClassMetadata
            {
                FNodeClassMetadata Metadata
                {
                    FNodeClassName{"TranspersonalGame", "DynamicMusicSystem", "Audio"},
                    1, // Major Version
                    0, // Minor Version
                    LOCTEXT("DisplayName", "Dynamic Music System"),
                    LOCTEXT("Description", "Sistema de música adaptativa que responde ao estado emocional do jogo"),
                    "Transpersonal Game Studio",
                    PluginNodeMissingPrompt,
                    GetVertexInterface(),
                    {NodeCategories::Music},
                    {},
                    FNodeDisplayStyle{}
                };
                return Metadata;
            };

            static const FNodeClassMetadata Metadata = CreateNodeClassMetadata();
            return Metadata;
        }

        FDynamicMusicSystemOperator(const FOperatorSettings& InSettings,
            const FFloatReadRef& InThreatLevel,
            const FFloatReadRef& InPlayerHeartRate,
            const FBoolReadRef& InIsDinosaur,
            const FFloatReadRef& InTimeOfDay,
            const FBoolReadRef& InIsHiding)
            : ThreatLevel(InThreatLevel)
            , PlayerHeartRate(InPlayerHeartRate)
            , IsDinosaur(InIsDinosaur)
            , TimeOfDay(InTimeOfDay)
            , IsHiding(InIsHiding)
            , MusicOutput(FAudioBufferWriteRef::CreateNew(InSettings))
            , IntensityLevel(FFloatWriteRef::CreateNew(0.0f))
            , ShouldTriggerStinger(FBoolWriteRef::CreateNew(false))
            , SampleRate(InSettings.GetSampleRate())
            , NumFramesPerBlock(InSettings.GetNumFramesPerBlock())
        {
        }

        virtual void Execute()
        {
            // Calcula intensidade baseada em múltiplos factores
            float CurrentIntensity = CalculateIntensity();
            *IntensityLevel = CurrentIntensity;

            // Gera música procedural baseada na intensidade
            GenerateAdaptiveMusic(CurrentIntensity);

            // Determina se deve tocar stinger de tensão
            *ShouldTriggerStinger = ShouldPlayStinger(CurrentIntensity);
        }

    private:
        float CalculateIntensity()
        {
            float BaseIntensity = *ThreatLevel;
            
            // Dinossauro próximo aumenta drasticamente a tensão
            if (*IsDinosaur)
            {
                BaseIntensity = FMath::Max(BaseIntensity, 0.8f);
            }

            // Batimento cardíaco influencia intensidade
            float HeartRateInfluence = FMath::Clamp((*PlayerHeartRate - 60.0f) / 100.0f, 0.0f, 1.0f);
            BaseIntensity += HeartRateInfluence * 0.3f;

            // Período noturno aumenta tensão base
            float DayNightCycle = *TimeOfDay;
            if (DayNightCycle < 6.0f || DayNightCycle > 20.0f) // Noite/madrugada
            {
                BaseIntensity += 0.2f;
            }

            // Esconder-se reduz ligeiramente a tensão mas mantém suspense
            if (*IsHiding)
            {
                BaseIntensity *= 0.8f;
                BaseIntensity += 0.1f; // Suspense de estar escondido
            }

            return FMath::Clamp(BaseIntensity, 0.0f, 1.0f);
        }

        void GenerateAdaptiveMusic(float Intensity)
        {
            FAudioBuffer& OutputBuffer = *MusicOutput;
            
            // Limpa o buffer
            OutputBuffer.Zero();

            // Gera música procedural baseada na intensidade
            for (int32 FrameIndex = 0; FrameIndex < NumFramesPerBlock; ++FrameIndex)
            {
                float Sample = GenerateMusicSample(Intensity, FrameIndex);
                OutputBuffer[FrameIndex] = Sample;
            }
        }

        float GenerateMusicSample(float Intensity, int32 FrameIndex)
        {
            // Frequências base para diferentes níveis de tensão
            float BaseFreq = FMath::Lerp(40.0f, 80.0f, Intensity); // Sub-bass para tensão
            float MidFreq = FMath::Lerp(200.0f, 800.0f, Intensity); // Frequências médias
            
            float Time = (CurrentSample + FrameIndex) / SampleRate;
            
            // Oscilador de baixa frequência para tensão
            float LowOsc = FMath::Sin(2.0f * PI * BaseFreq * Time) * (0.3f + Intensity * 0.4f);
            
            // Ruído filtrado para atmosfera
            float Noise = (FMath::RandRange(-1.0f, 1.0f)) * 0.1f * Intensity;
            
            // Harmônicos para complexidade
            float Harmonics = 0.0f;
            for (int32 i = 2; i <= 4; ++i)
            {
                Harmonics += FMath::Sin(2.0f * PI * BaseFreq * i * Time) * (0.1f / i) * Intensity;
            }

            return (LowOsc + Noise + Harmonics) * 0.3f;
        }

        bool ShouldPlayStinger(float Intensity)
        {
            // Trigger stinger quando intensidade sobe rapidamente
            static float LastIntensity = 0.0f;
            float IntensityDelta = Intensity - LastIntensity;
            LastIntensity = Intensity;

            return IntensityDelta > 0.3f && Intensity > 0.6f;
        }

        FFloatReadRef ThreatLevel;
        FFloatReadRef PlayerHeartRate;
        FBoolReadRef IsDinosaur;
        FFloatReadRef TimeOfDay;
        FBoolReadRef IsHiding;

        FAudioBufferWriteRef MusicOutput;
        FFloatWriteRef IntensityLevel;
        FBoolWriteRef ShouldTriggerStinger;

        float SampleRate;
        int32 NumFramesPerBlock;
        int64 CurrentSample = 0;
    };

    METASOUND_REGISTER_NODE(FDynamicMusicSystemOperator)
}

#undef LOCTEXT_NAMESPACE